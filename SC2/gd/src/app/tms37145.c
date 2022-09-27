// When the protection bit is set, programming to the corresponding memory Page is only possible with mutual authentication programming. 


#include "headfile.h"

tms37145_tag_t tms37145_tag1;
tms37145_tag_t tms37145_tag_copy;

uint16_t tms37145_calc_crc16(const uint8_t *p_data, uint8_t bytes)
{
	uint8_t i;
	uint16_t crc;

	crc = 0x3791;				// ????
	while (bytes--) {
		crc ^= *p_data++;
		for (i = 0; i < 8; i++) {
			if (crc & 0x0001) {
				crc >>= 1;
				crc ^= 0x8408;
			}
			else
				crc >>= 1;
		}
	}

	return crc;
}

#define TMS37145_T_CHARGE					(50)
#define TMS37145_T_RESET					(15)
#define TMS37145_T_PROG						(15 + 5)
#define TMS37145_T_ENC						(4 + 4)
#define TMS37145_T_MUT						(8 + 8)
#define TMS37145_T_PWM_READ_RECH			(0 + 8)

#define TMS37145_T_HDET_6					(356)
#define TMS37145_T_HDET_8					(476)

#if _INIT_TS_TAG
/* optimize for TS4D */
#define TMS37145_PWM_T_OFF_L				(160)
#define TMS37145_PWM_T_ON_L					(340)
#define TMS37145_PWM_T_OFF_H				(650)
#define TMS37145_PWM_T_ON_H					(850)
#else
/* TMS37145 datasheet time */
#define TMS37145_PWM_T_OFF_L				(170)
#define TMS37145_PWM_T_ON_L					(330)
#define TMS37145_PWM_T_OFF_H				(480)
#define TMS37145_PWM_T_ON_H					(520)
#endif

#define TMS37145_PPM6_T_OFF					(120)
#define TMS37145_PPM6_T_ON_L				(300 - TMS37145_PPM6_T_OFF)
#define TMS37145_PPM6_T_ON_H				(420 - TMS37145_PPM6_T_OFF)
#define TMS37145_PPM8_T_OFF					(170)
#define TMS37145_PPM8_T_ON_L				(400 - TMS37145_PPM6_T_OFF)
#define TMS37145_PPM8_T_ON_H				(520 - TMS37145_PPM6_T_OFF)

void tms37145_pwm_wr_lsbit(const uint8_t *p_wr, uint8_t bits, uint8_t recharge_ms)
{
	uint8_t i, byte=0;

	for (i = 0; i < bits; i++) {
		if ((i % 8) == 0)
			byte = *p_wr++;

		tms3705_driver_off();
		if (byte & 0x01)
			delay_us(TMS37145_PWM_T_OFF_H - 1);
		else
			delay_us(TMS37145_PWM_T_OFF_L - 1);
		
		tms3705_driver_on();
		if (byte & 0x01)
			delay_us(TMS37145_PWM_T_ON_H - 1);
		else
			delay_us(TMS37145_PWM_T_ON_L - 1);
		
		byte >>= 1;
	}
        

	delay_us((uint16_t)recharge_ms*1000);
	tms3705_driver_off();
}

void tms37145_ppm6_wr_lsbit(const uint8_t *p_wr, uint8_t bits, uint8_t recharge_ms)
{
	uint8_t i, byte=0;

	for (i = 0; i < bits; i++) {
		if ((i % 8) == 0)
			byte = *p_wr++;

		tms3705_driver_off();
		timer1_stop();
		TCNT1 = /*(65536 - */TMS37145_PPM6_T_OFF*2-2 /*+ 2)*/;	// +2 ensure accurate time
		timer1_clear_ov();
		timer1_start_8();
		while (timer1_ov_flag == 0);

		tms3705_driver_on();
		timer1_stop();
		TCNT1 = (byte & 0x01) ? (/*65536 - */TMS37145_PPM6_T_ON_H*2-2)/* + 2)*/ : (/*65536 -*/ TMS37145_PPM6_T_ON_L*2-2)/* + 2)*/;
		timer1_clear_ov();
		timer1_start_8();
		while (timer1_ov_flag == 0);

		timer1_stop();
		timer1_clear_ov();
		
		byte >>= 1;
	}

	if (recharge_ms > 0) {
		tms3705_driver_off();
		timer1_stop();
		TCNT1 = (/*65536 -*/ TMS37145_PPM6_T_OFF*2-2)/* + 2)*/;	// +2 ensure accurate time
		timer1_clear_ov();
		timer1_start_8();
		while (timer1_ov_flag == 0);

		timer1_stop();
		timer1_clear_ov();
		tms3705_driver_on();
		delay_us((uint16_t)recharge_ms*1000);
	}
	tms3705_driver_off();
}

void tms37145_ppm8_wr_lsbit(const uint8_t *p_wr, uint8_t bits, uint8_t recharge_ms)
{
	uint8_t i, byte=0;

	for (i = 0; i < bits; i++) {
		if ((i % 8) == 0)
			byte = *p_wr++;

		tms3705_driver_off();
		timer1_stop();
		TCNT1 = (/*65536 -*/ TMS37145_PPM8_T_OFF*2-2 )/*+ 2)*/;	// +2 ensure accurate time
		timer1_clear_ov();
		timer1_start_8();
		while (timer1_ov_flag == 0);

		tms3705_driver_on();
		timer1_stop();
		TCNT1 = (byte & 0x01) ? (/*65536 -*/ TMS37145_PPM8_T_ON_H*2-2)/* + 2) */
								: (/*65536 -*/ TMS37145_PPM8_T_ON_L*2-2)/* + 2)*/;
		timer1_clear_ov();
		timer1_start_8();
		while (timer1_ov_flag == 0);

		timer1_stop();
		timer1_clear_ov();
		
		byte >>= 1;
	}

	if (recharge_ms > 0) {
		tms3705_driver_off();
		timer1_stop();
		TCNT1 = (/*65536 - */TMS37145_PPM8_T_OFF*2-2)/* + 2)*/;	// +2 ensure accurate time
		timer1_clear_ov();
		timer1_start_8();
		while (timer1_ov_flag == 0);

		timer1_stop();
		timer1_clear_ov();
		tms3705_driver_on();
		delay_us((uint16_t)recharge_ms*1000);
	}
	tms3705_driver_off();
}

uint8_t tms37145_pwm_wr_ls_rd(uint8_t bytes_wr, const uint8_t *p_wr, uint8_t recharge_ms, uint8_t *p_rd)
{
	uint8_t result;
	uint8_t a_read[10];

	result = FALSE;
	if (tms3705_init_charge(0x00, TMS37145_T_CHARGE)) {
		tms37145_pwm_wr_lsbit(p_wr, bytes_wr*8, recharge_ms);
		if (tms3705_rd_data(a_read, 10)) {
			copy_bytes(p_rd, a_read, 10);
			result = TRUE;
		}
	}

	delay_us(TMS37145_T_RESET*1000);
	return result;
}

uint8_t tms37145_pwm_wr_ls_crc_rd(uint8_t bytes_wr, const uint8_t *p_wr, uint8_t recharge_ms, uint8_t *p_rd)
{
	uint8_t a_crc[2];
	uint16_t crc;
	uint8_t result;
	uint8_t a_read[10];
	crc = tms37145_calc_crc16(p_wr, bytes_wr);
	a_crc[0] = (uint8_t)crc;
	a_crc[1] = (uint8_t)(crc>>8);

	result = FALSE;
	if (tms3705_init_charge(0, TMS37145_T_CHARGE)) {
		tms37145_pwm_wr_lsbit(p_wr, bytes_wr*8, 0);
		tms37145_pwm_wr_lsbit(a_crc, 16, recharge_ms);
		if (tms3705_rd_data(a_read, 10)) {
			copy_bytes(p_rd, a_read, 10);
			result = TRUE;
		}
	}

	delay_us(TMS37145_T_RESET*1000);
	return result;
}

uint8_t tms37145_pwmtime_wr_ls_rd(uint8_t bytes_wr, const uint8_t *p_wr, uint8_t recharge_ms, uint8_t *p_rd)
{
	uint8_t result;
	uint8_t a_read[10];

	result = FALSE;
	if (tms3705_init_charge(0, TMS37145_T_CHARGE)) {
		tms3705_pwm_wr_lsbit(p_wr, bytes_wr*8, recharge_ms);
		if (tms3705_rd_data(a_read, 10)) {
			copy_bytes(p_rd, a_read, 10);
			result = TRUE;
		}
	}

	return result;
}

uint8_t tms37145_pwmtime_wr_ls_crc_rd(uint8_t bytes_wr, const uint8_t *p_wr, uint8_t recharge_ms, uint8_t *p_rd)
{
	uint8_t a_crc[2];
	uint16_t crc;
	uint8_t result;
	uint8_t a_read[10];
	crc = tms37145_calc_crc16(p_wr, bytes_wr);
	a_crc[0] = (uint8_t)crc;
	a_crc[1] = (uint8_t)(crc>>8);


	result = FALSE;
	if (tms3705_init_charge(0, TMS37145_T_CHARGE) == TRUE) {
		tms3705_pwm_wr_lsbit(p_wr, bytes_wr*8, 0);
		tms3705_pwm_wr_lsbit(a_crc, 16, recharge_ms);
		if (tms3705_rd_data(a_read, 10) == TRUE) {
			copy_bytes(p_rd, a_read, 10);
			result = TRUE;
		}
	}

	return result;
}

uint8_t tms37145_ppm_wr_ls_rd(uint8_t bytes_wr, const uint8_t *p_wr, uint8_t recharge_ms, uint8_t *p_rd)
{
	uint8_t a_read[10];

	/* ppm6 case */
	if (!tms3705_init_charge(0, TMS37145_T_CHARGE))
		return FALSE;
	tms37145_ppm6_wr_lsbit(p_wr, bytes_wr*8, recharge_ms);
	if (tms3705_rd_data(a_read, 10)) {
		copy_bytes(p_rd, a_read, 10);
		return TRUE;
	}

	/* ppm8 case */
	if (!tms3705_init_charge(0, TMS37145_T_CHARGE))
		return FALSE;
	tms37145_ppm8_wr_lsbit(p_wr, bytes_wr*8, recharge_ms);
	if (tms3705_rd_data(a_read, 10)) {
		copy_bytes(p_rd, a_read, 10);
		return TRUE;
	}

	return FALSE;
}

uint8_t tms37145_ppm_wr_ls_crc_rd(uint8_t bytes_wr, const uint8_t *p_wr, uint8_t recharge_ms, uint8_t *p_rd)
{
	uint8_t a_crc[2];
	uint16_t crc;
	uint8_t a_read[10];
	crc = tms37145_calc_crc16(p_wr, bytes_wr);
	a_crc[0] = (uint8_t)crc;
	a_crc[1] = (uint8_t)(crc>>8);


	/* ppm6 case */
	if (!tms3705_init_charge(0, TMS37145_T_CHARGE))
		return FALSE;
	tms37145_ppm6_wr_lsbit(p_wr, bytes_wr*8, 0);
	tms37145_ppm6_wr_lsbit(a_crc, 16, recharge_ms);
	if (tms3705_rd_data(a_read, 10)) {
		copy_bytes(p_rd, a_read, 10);
		return TRUE;
	}

	/* ppm8 case */
	if (!tms3705_init_charge(0, TMS37145_T_CHARGE))
		return FALSE;
	tms37145_ppm8_wr_lsbit(p_wr, bytes_wr*8, 0);
	tms37145_ppm8_wr_lsbit(a_crc, 16, recharge_ms);
	if (tms3705_rd_data(a_read, 10)) {
		copy_bytes(p_rd, a_read, 10);
		return TRUE;
	}

	return FALSE;
}

uint8_t tms37145_ppmtime_wr_ls_rd(uint8_t bytes_wr, const uint8_t *p_wr, uint8_t recharge_ms, uint8_t *p_rd)
{
	uint8_t a_read[10];

	if (!tms3705_init_charge(0, TMS37145_T_CHARGE))
		return FALSE;
	tms3705_ppm_wr_lsbit(p_wr, bytes_wr*8, recharge_ms);
	if (tms3705_rd_data(a_read, 10)) {
		copy_bytes(p_rd, a_read, 10);
		return TRUE;
	}

	return FALSE;
}

uint8_t tms37145_ppmtime_wr_ls_crc_rd(uint8_t bytes_wr, const uint8_t *p_wr, uint8_t recharge_ms, uint8_t *p_rd)
{
	uint8_t a_crc[2];
	uint16_t crc;
uint8_t a_read[10];
	crc = tms37145_calc_crc16(p_wr, bytes_wr);
	a_crc[0] = (uint8_t)crc;
	a_crc[1] = (uint8_t)(crc>>8);



	if (!tms3705_init_charge(0, TMS37145_T_CHARGE))
		return FALSE;
	tms3705_ppm_wr_lsbit(p_wr, bytes_wr*8, 0);
	tms3705_ppm_wr_lsbit(a_crc, 16, recharge_ms);
	if (tms3705_rd_data(a_read, 10)) {
		copy_bytes(p_rd, a_read, 10);
		return TRUE;
	}

	return FALSE;
}

uint8_t tms37145_lock_page(uint8_t page, uint8_t psw)
{
		uint8_t i, result;
	uint8_t a_wr[2], a_rd[10];
	switch (page) {
		case 1:
		case 2:
		case 3:
		case 4:
			break;
		default:
			return FALSE;
	}
	
	
	result = FALSE;
	i = 1;
	a_wr[0] = page*4 + 2;
	if (psw != 0xFF)
		a_wr[i++] = psw;
	if (tms37145_pwm_wr_ls_crc_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
		result = TRUE;

	return result;
}

uint8_t tms37145_check_dst40(tms37145_tag_t *p_tag)
{
	/* staitc local variable store in RAM; auto local variable store in stack.
	use "static" avoid stack overflow */
	static uint64_t xor1, xor_key, xor2;
	
	uint8_t a_random[5], a_skey[5];
	uint32_t sign;
	
	p_tag->crack_flag = FALSE;
	switch (p_tag->page1[0]) {
		case 0x32:  //67 tag
		case 0x52:
		case 0x72:
		case 0x92:
		case 0xB2:
			xor_key = 0x3523352335;	//const, come from other
			p_tag->crack_type = 0x67;
			break;
		case 0x30:  //68 tag
		case 0x50:
		case 0x70:
		case 0x90:
		case 0xB0:
			xor_key = 0x0310031003;	//const, come from other
			p_tag->crack_type = 0x68;
			break;
		case 0x33:  //70 tag
		case 0x53:
		case 0x73:
		case 0x93:
		case 0xB3:
			xor_key = 0x4634463446;	//const, come from other
			p_tag->crack_type = 0x70;
			break;
		default:
			return FALSE;
	}
	xor1 = (((uint64_t)p_tag->page3[0]) << 32) | (((uint64_t)p_tag->page3[1]) << 24)
			 | (((uint64_t)p_tag->page3[2]) << 16) | (((uint64_t)p_tag->page1[0]) << 8)
			 | ((uint64_t)p_tag->page2[0]);
	xor2 = (((uint64_t)p_tag->page2[0]) << 32) | (((uint64_t)p_tag->page2[0]) << 24)
			 | (((uint64_t)p_tag->page2[0]) << 16) | (((uint64_t)p_tag->page2[0]) << 8) 
			 | ((uint64_t)p_tag->page2[0]);

	xor1 ^= xor_key;
	xor1 <<= 1;
	xor1 ^= xor2;

	a_skey[4] = (uint8_t)(xor1 >> 32);	// MSByte key
	a_skey[3] = (uint8_t)(xor1 >> 24);
	a_skey[2] = (uint8_t)(xor1 >> 16);
	a_skey[1] = (uint8_t)(xor1 >> 8);
	a_skey[0] = (uint8_t)xor1;
	p_tag->page4[0] = a_skey[4];		// MSByte key
	p_tag->page4[1] = a_skey[3];
	p_tag->page4[2] = a_skey[2];
	p_tag->page4[3] = a_skey[1];
	p_tag->page4[4] = a_skey[0];

	/* use one of chanllge random before send(see find tag function signature 3) */
	a_random[0]=0x16;
	a_random[1]=0xA0;
	a_random[2]=0x4B;
	a_random[3]=0x25;
	a_random[4]=0x75;
	/* see find tag function signature 3 */
	sign = ((uint32_t)p_tag->signature[2][2] << 16)
			| ((uint32_t)p_tag->signature[2][1] << 8)
			| ((uint32_t)p_tag->signature[2][0]);

	/* use dst40 algorithm and compare result */
	if (sign == dst40_cycle(a_random, a_skey, 200)) {
		p_tag->crack_flag = TRUE;
		return TRUE;
	}
	else
		return FALSE;
}

#define CRACK_DST40_MAX_TIME				(180)
/*uint8_t tms37145_crack_dst40(tms37145_tag_t *p_tag)
{
	uint8_t i, result;*/
//	uint8_t /*a_send[5],*/ a_rece[LFE3_RECE_4D_LEN];
/*	
	for (i = 0; i < (LFE3_INIT_SECOND*10); i++) {
		if (t1_tick_2s >= LFE3_INIT_SECOND/2)
			break;
		delay_us(100*1000);
	}
*/
//	INTX_DISABLE();
	//lfe3_start_clock();
//	a_send[0] = 0xAA;
	//a_send[1] = 0xBB;
//	a_send[2] = 0x4D;
	//lfe3_send_data(a_send, 3);
	//lfe3_send_data(p_tag->signature[0], 3);
	//lfe3_send_data(p_tag->signature[1], 3);
	//lfe3_send_data(p_tag->signature[2], 3);
	//lfe3_send_data(p_tag->signature[3], 3);
	//a_send[0] = 0x00;
	//a_send[1] = 0x55;
	//lfe3_send_data(a_send, 2);
/*	INTX_ENABLE();

	#if DISPLAY_CRACK_TIME
	while (t2_tick_1s < CRACK_DST40_MAX_TIME) {
	//	if (lfe3_sta_status) {
		//	delay_us(500);							// remove shake
	//		if (lfe3_sta_status)
	//			break;
	//	}
	}
	if (t2_tick_1s < CRACK_DST40_MAX_TIME) {
		delay_us(2500*1000);
		INTX_DISABLE();
		//result = lfe3_rece_data(a_rece, LFE3_RECE_4D_LEN);
		INTX_ENABLE();
	}
	else
		result = FALSE;
	#else
	INTX_DISABLE(();
	//result = lfe3_rece_data(a_rece, LFE3_RECE_4D_LEN);
	INTX_ENABLE();
	#endif

	INTX_DISABLE();
	//lfe3_stop_clock();
	INTX_ENABLE();

	if (result)
		copy_bytes(p_tag->page4, &a_rece[3], 5);			// MSByte-->LSByte
	
	return result;
}*/

uint8_t tms37145_compare_data(const tms37145_tag_t *p_tag1, const tms37145_tag_t *p_tag2)
{
	if (p_tag1->read_len != p_tag2->read_len)
		return FALSE;
	if (!equal_bytes(p_tag1->page1, p_tag2->page1, 1))
		return FALSE;
	if (!equal_bytes(p_tag1->page2, p_tag2->page2, 1))
		return FALSE;
	if (!equal_bytes(p_tag1->page3, p_tag2->page3, 4))
		return FALSE;
	if (!equal_bytes(&(p_tag1->signature[0][0]), &(p_tag2->signature[0][0]), 3))
		return FALSE;
	if (!equal_bytes(&(p_tag1->signature[1][0]), &(p_tag2->signature[1][0]), 3))
		return FALSE;

	return TRUE;
}

bool tms37145_find_tag(tms37145_tag_t *p_tag)
{
	uint8_t a_wr[8], a_rd[12];
	bool result = FALSE;
	a_wr[0] = 0x78;
	a_wr[1] = 0x04;
	a_rd[0] = 0x00;
	a_rd[7] = 0x00;
	
	p_tag->crack_flag = FALSE;
	p_tag->crack_type = 0x4D;
	p_tag->mode = e_tms37145_not;
	p_tag->read_len = 10;
	tms37145_pwm_wr_ls_rd(1, &a_wr[0], TMS37145_T_PWM_READ_RECH, a_rd);
	/* remove a_rd[7] = 0x18 case */
	if ((a_rd[0] == 0x7E) && ((a_rd[7]&0xF0) == 0x70)) {
		p_tag->mode = e_tms37145_pwm;
		p_tag->page30[1] = a_rd[1];		// LSByte
		p_tag->page30[0] = a_rd[2];		// MSByte
		if (((a_rd[1]&0x0E) == 0x00) && ((a_rd[2]&0x50) == 0x40))
			p_tag->mode = e_tms37145_b9wk;

		/* because TS0D have 64 mode */
		if (!tms37145_pwm_wr_ls_rd(1, &a_wr[1], TMS37145_T_PWM_READ_RECH, a_rd))
			;
		else if (a_rd[0] != 0x7E)
			;
		else if (tms37145_calc_crc16(&a_rd[1], 5) == (a_rd[6] | ((uint16_t)a_rd[7]<<8))) {
			p_tag->mode = e_b9wk;
			p_tag->read_len = 8;
		}
	}
	else if (tms37145_pwm_wr_ls_rd(1, &a_wr[1], TMS37145_T_PWM_READ_RECH, a_rd)) {
		if (a_rd[0] != 0x7E)
			return FALSE;
		else if (tms37145_calc_crc16(&a_rd[1], 5) == (a_rd[6] | ((uint16_t)a_rd[7]<<8))) {
			p_tag->mode = e_b9wk;
			p_tag->read_len = 8;
		}
		else if (tms37145_calc_crc16(&a_rd[1], 7) == (a_rd[8] | ((uint16_t)a_rd[9]<<8))) {
			p_tag->mode = e_b9wk;
			p_tag->read_len = 10;
		}
	}
	else if (tms37145_ppm_wr_ls_rd(1, &a_wr[0], 0, a_rd)) {
		if (a_rd[0] != 0x7E)
			return FALSE;
		else {
			p_tag->mode = e_tms37145_ppm;
			p_tag->page30[1] = a_rd[1];
			p_tag->page30[0] = a_rd[2];
		}
	}
	else
		return FALSE;
	
	while ((p_tag->mode == e_b9wk) || (p_tag->mode == e_tms37145_b9wk) || (p_tag->mode == e_tms37145_pwm)) {
		/* page1 */
		a_wr[0] = 0x04;
		if (!tms37145_pwm_wr_ls_rd(1, &a_wr[0], TMS37145_T_PWM_READ_RECH, a_rd)) {
			p_tag->mode = e_tms37145_not;
			break;
		}
		else if (p_tag->read_len == 10) {
			p_tag->page1[0] = a_rd[1];
			p_tag->page1[1] = (a_rd[7]==a_wr[0]) ? e_page_unlock : e_page_lock;
		}
		else {
			p_tag->page1[0] = a_rd[1];
			p_tag->page1[1] = (a_rd[5]==a_wr[0]) ? e_page_unlock : e_page_lock;
		}
		
		/* page2 */
		a_wr[0] = 0x08;
		if (!tms37145_pwm_wr_ls_rd(1, &a_wr[0], TMS37145_T_PWM_READ_RECH, a_rd)) {
			p_tag->mode = e_tms37145_not;
			break;
		}
		else if (p_tag->read_len == 10) {
			p_tag->page2[0] = a_rd[2];
			p_tag->page2[1] = (a_rd[7]==a_wr[0]) ? e_page_unlock : e_page_lock;
		}
		else {
			p_tag->page2[0] = a_rd[1];
			p_tag->page2[1] = (a_rd[5]==a_wr[0]) ? e_page_unlock : e_page_lock;
		}
		
		/* page3 */
		a_wr[0] = 0x0C;
		if (!tms37145_pwm_wr_ls_rd(1, &a_wr[0], TMS37145_T_PWM_READ_RECH, a_rd)) {
			(p_tag->mode = e_tms37145_not);
			break;
		}
		else if (p_tag->read_len == 10) {
			p_tag->page3[3] = a_rd[3];
			p_tag->page3[2] = a_rd[4];
			p_tag->page3[1] = a_rd[5];
			p_tag->page3[0] = a_rd[6];
			p_tag->page3[4] = (a_rd[7]==a_wr[0]) ? e_page_unlock : e_page_lock;
		}
		else {
			p_tag->page3[3] = a_rd[1];
			p_tag->page3[2] = a_rd[2];
			p_tag->page3[1] = a_rd[3];
			p_tag->page3[0] = a_rd[4];
			p_tag->page3[4] = (a_rd[5]==a_wr[0]) ? e_page_unlock : e_page_lock;
		}

		/* decide not b9wk */
		if (p_tag->mode == e_tms37145_pwm) {
			result = TRUE;
			break;
		}

		/* signature 1 */
		a_wr[0] = 0x10;
		a_wr[1] = 0x8B;
		a_wr[2] = 0xF3;
		a_wr[3] = 0x5F;
		a_wr[4] = 0x27;
		a_wr[5] = 0x57;
		if (!tms37145_pwm_wr_ls_rd(6, &a_wr[0], TMS37145_T_ENC, a_rd)) {
			p_tag->mode = e_tms37145_not;
			break;
		}
		else if (p_tag->read_len == 10) {
			p_tag->signature[0][0] = a_rd[4];
			p_tag->signature[0][1] = a_rd[5];
			p_tag->signature[0][2] = a_rd[6];
			p_tag->page4[5] = (a_rd[7]==a_wr[0]) ? e_page_unlock : e_page_lock;
		}
		else {
			p_tag->signature[0][0] = a_rd[2];
			p_tag->signature[0][1] = a_rd[3];
			p_tag->signature[0][2] = a_rd[4];
			p_tag->page4[5] = (a_rd[5]==a_wr[0]) ? e_page_unlock : e_page_lock;
		}

		/*  signature 2 */
		a_wr[1] = 0x94;
		a_wr[2] = 0x77;
		a_wr[3] = 0x15;
		a_wr[4] = 0x46;
		a_wr[5] = 0x2D;
		if (!tms37145_pwm_wr_ls_rd(6, &a_wr[0], TMS37145_T_ENC, a_rd)) {
			p_tag->mode = e_tms37145_not;
			break;
		}
		else if (p_tag->read_len == 10) {
			p_tag->signature[1][0] = a_rd[4];
			p_tag->signature[1][1] = a_rd[5];
			p_tag->signature[1][2] = a_rd[6];
		}
		else {
			p_tag->signature[1][0] = a_rd[2];
			p_tag->signature[1][1] = a_rd[3];
			p_tag->signature[1][2] = a_rd[4];
		}

		/* signature 3 */
		a_wr[1] = 0x16;
		a_wr[2] = 0xA0;
		a_wr[3] = 0x4B;
		a_wr[4] = 0x25;
		a_wr[5] = 0x75;
		if (!tms37145_pwm_wr_ls_rd(6, &a_wr[0], TMS37145_T_ENC, a_rd)) {
			p_tag->mode = e_tms37145_not;
			break;
		}
		else if (p_tag->read_len == 10) {
			p_tag->signature[2][0] = a_rd[4];
			p_tag->signature[2][1] = a_rd[5];
			p_tag->signature[2][2] = a_rd[6];
		}
		else {
			p_tag->signature[2][0] = a_rd[2];
			p_tag->signature[2][1] = a_rd[3];
			p_tag->signature[2][2] = a_rd[4];
		}

		/* signature 4 */
		a_wr[1] = 0x02;
		a_wr[2] = 0x14;
		a_wr[3] = 0x12;
		a_wr[4] = 0x12;
		a_wr[5] = 0x20;
		if (!tms37145_pwm_wr_ls_rd(6, &a_wr[0], TMS37145_T_ENC, a_rd)) {
			p_tag->mode = e_tms37145_not;
			break;
		}
		else if (p_tag->read_len == 10) {
			p_tag->signature[3][0] = a_rd[4];
			p_tag->signature[3][1] = a_rd[5];
			p_tag->signature[3][2] = a_rd[6];
		}
		else {
			p_tag->signature[3][0] = a_rd[2];
			p_tag->signature[3][1] = a_rd[3];
			p_tag->signature[3][2] = a_rd[4];
		}

		tms37145_check_dst40(p_tag);
		result = TRUE;
		break;
	}

	while (p_tag->mode == e_tms37145_ppm) {
		/* page 1 */
		a_wr[0] = 0x04;
		if (tms37145_ppm_wr_ls_rd(1, &a_wr[0], 0, a_rd)) {
			p_tag->page1[0] = a_rd[1];
			p_tag->page1[1] = (a_rd[7]==a_wr[0]) ? e_page_unlock : e_page_lock;
		}
		else {
			p_tag->mode = e_tms37145_not;
			break;
		}
		
		/* page 2 */
		a_wr[0] = 0x08;
		if (tms37145_ppm_wr_ls_rd(1, &a_wr[0], 0, a_rd)) {
			p_tag->page2[0] = a_rd[2];
			p_tag->page2[1] = (a_rd[7]==a_wr[0]) ? e_page_unlock : e_page_lock;
		}
		else {
			p_tag->mode = e_tms37145_not;
			break;
		}
		
		/* page 3 */
		a_wr[0] = 0x0C;
		if (tms37145_ppm_wr_ls_rd(1, &a_wr[0], 0, a_rd)) {
			p_tag->page3[3] = a_rd[3];
			p_tag->page3[2] = a_rd[4];
			p_tag->page3[1] = a_rd[5];
			p_tag->page3[0] = a_rd[6];
			p_tag->page3[4] = (a_rd[7]==a_wr[0]) ? e_page_unlock : e_page_lock;
		}
		else {
			p_tag->mode = e_tms37145_not;
			break;
		}

		result = TRUE;
		break;
	}

	return result;
}

bool tms37145_keypad_interaction(tms37145_tag_t *p_tag)
{
		//uint8_t /*key,*/ a_display[16];
	/*	enum {
		s_show_id,
		s_crack, s_crack_ok, s_crack_ng,
		s_copying, s_copy_ok, s_copy_ng,
		s_exit
	} state;*/
	if (p_tag->mode == e_tms37145_not)
		return FALSE;

/*
	hex_to_char(p_tag->page1, &a_display[0], 1);
	a_display[2] = ' ';
	hex_to_char(p_tag->page2, &a_display[3], 1);
	a_display[5] = ' ';
	hex_to_char(p_tag->page3, &a_display[6], 3);
	a_display[12] = ' ';
	hex_to_char(&(p_tag->page3[3]), &a_display[13], 1);*/
	
	//state = s_show_id;
	//lcd_clear_screen();
	//LCD_Fill(10,50,lcddev.width,200,BACK_COLOR);
		//LCD_Fill(10,50,lcddev.width,lcddev.height,BACK_COLOR);
		OLED_Clear( );
	if ((p_tag->mode == e_b9wk) || (p_tag->mode == e_tms37145_b9wk)) {
		if (p_tag->crack_flag) {
			oled_print_hex(0,0,16,&(p_tag->crack_type), 1);
			//lcd_print_hex(10,50,200,16,16, &(p_tag->crack_type), 1);
			//LCD_ShowString(26,50,200,16,16, str_tag, 3);
			OLED_ShowString(0,19,str_tag, 16,3);
			oled_print_hex(0,46,16,p_tag->page4, 5);
			//lcd_print_hex(74,50,200,16,16, p_tag->page4, 5);
		}
		else if (p_tag->mode == e_tms37145_b9wk&&p_tag->page3[3] != 0x01) {
			OLED_ShowString(0, 0, str_40, 16,6);
			//LCD_ShowString(10, 50,200,16,16, str_40, 6);
		}
		else if (p_tag->read_len == 8)
			OLED_ShowString(0, 0,str_64_tag,16,6);
			//LCD_ShowString(10, 50,200,16,16, str_64_tag, 6);
		else if (p_tag->page3[3] == 0x01)
			  OLED_ShowString(0, 0,str_63_tag, 16,6);
			//LCD_ShowString(10, 50,200,16,16, str_63_tag, 6);
		else if (p_tag->read_len == 10)
			OLED_ShowString(0, 0,str_4d_tag, 16,6);
			//LCD_ShowString(10, 50,200,16,16, str_4d_tag, 6);

		//while (state != s_exit) {
			//switch (state) {
			//	case s_show_id:
					//delay_us(T_DISPLAY*1000);
				//	LCD_ShowString(10,70,200,16,16, a_display, 15);
					//LCD_ShowString(0, 1, str_exit_r, 6);
					//if (p_tag->crack_flag)
						//lcd_print_str(8, 1, str_copy_c, 6);
					//else
						//LCD_ShowString(10,90,200,16,16, str_crack_c, 7);

				//	while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
					//if (key == KEYPAD_FLAG_READ)
						//state = s_exit;
					//else if (p_tag->crack_flag)
						//state = s_copying;
					//state = s_exit;
					//else
						//state = s_crack;
				//	break;
				//case s_crack:
					//lcd_print_cracking();
					//if (tms37145_crack_dst40(p_tag)){
						//state = s_crack_ok;
					//state = s_exit;
					//LCD_ShowString(10, 90,200,16,16, p_tag->page4, 5);
					//}
				//	else
					//{	//state = s_crack_ng;
					//state = s_crack;
					//LCD_ShowString(10,90,200,16,16, str_crack_no, 8);
				//	}
				//	break;
				//case s_crack_ok:
					//lcd_print_hex(30, 40,200,24,24, p_tag->page4, 5);
					/*lcd_print_str(0, 1, str_exit_r, 6);
					lcd_print_str(8, 1, str_copy_c, 6);
					while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
					if (key == KEYPAD_FLAG_READ)*/
						//state = s_exit;
					//else
						//state = s_copying;
				//	break;
				//case s_crack_ng:
					//lcd_print_str(0, 0, str_crack_no, 8);
					//lcd_print_str(0, 1, str_exit_r, 6);
					//lcd_print_str(8, 1, str_crack_c, 7);
				//	while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
					//if (key == KEYPAD_FLAG_READ)
					//	state = s_exit;
					//else
				//		state = s_crack;
				//	break;
				/*case s_copying:
					lcd_clear_screen();
					lcd_print_str(0, 0, str_insert, 6);
					lcd_print_str(8, 0, str_new_4d, 6);
					lcd_print_str(0, 1, str_exit_r, 6);
					lcd_print_str(8, 1, str_copy_c, 6);
					while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
					if (key == KEYPAD_FLAG_READ)
						state = s_exit;
					else {
						lcd_print_copying();
						state = s_copy_ok;
						if (ts804d_copy_b9wk(p_tag))
							;
						else if (tpx2_copy_b9wk(p_tag))
							;
					#if 0
						//V2.25B, leader ChengZQ popularize TS Tag
						else if (ys01_copy_b9wk(p_tag))
							;
						else if (ts0d_copy_b9wk(p_tag))
							;
						else if (ys80_copy_b9wk(p_tag))
							;
					#endif
						else if (ts0dnew_copy_b9wk(p_tag))
							;
						else
							state = s_copy_ng;
					}
					break;
				case s_copy_ok:
				case s_copy_ng:
					lcd_clear_screen();
					if (state == s_copy_ok)
						lcd_print_str(0, 0, str_copy_ok, 8);
					else
						lcd_print_str(0, 0, str_copy_ng, 8);
					lcd_print_str(0, 1, str_exit_r, 6);
					lcd_print_str(8, 1, str_copy_c, 6);
					while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
					if (key == KEYPAD_FLAG_READ)
						state = s_exit;
					else
						state = s_copying;
					break;*/
			//	case s_exit:
			//	default :
				//	state = s_exit;
				//	break;
		//	}
		//}
	}
	else if ((p_tag->mode == e_tms37145_pwm) || (p_tag->mode == e_tms37145_ppm)) {
		OLED_ShowString(0, 0, str_80, 18,6);
		//LCD_ShowString(10,50,200,16,16, str_80, 6);
		/*while (state != s_exit) {
			switch (state) {
				case s_show_id:
					//delay_us(T_DISPLAY*1000);
					LCD_ShowString(10,70,200,16,16, a_display, 15);
					//lcd_print_str(0, 1, str_exit_r, 6);
				//	while ((key = keypad_scan()) != KEYPAD_FLAG_READ);
					state = s_exit;
					break;
				case s_exit:
				default :
					state = s_exit;
					break;
			}
		}*/
	}
	else
		return FALSE;
	
	return TRUE;
}

uint8_t b9wk_set_tag(const tms37145_tag_t *p_b9wk, uint8_t lock_flag)
{
	uint8_t i, result, psw;
	uint8_t a_wr[8], a_rd[10];

	a_wr[0] = 0x04;
	if (tms37145_pwm_wr_ls_rd(1, a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
		psw = a_rd[1];
	else
		return FALSE;

	/* write page1 */
	result = FALSE;
	i = 0;
	a_wr[i++] = 0x05;
	if (psw != 0xFF)
		a_wr[i++] = psw;
	a_wr[i++] = p_b9wk->page1[0];
	if (tms37145_pwm_wr_ls_crc_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
		if (a_rd[7] == a_wr[0]) {
			result = TRUE;
			psw = a_rd[1];
		}

	/* write page2 */
	if (result) {
		result = FALSE;
		i = 0;
		a_wr[i++] = 0x09;
		if (psw != 0xFF)
			a_wr[i++] = psw;
		a_wr[i++] = p_b9wk->page2[0];
		if (tms37145_pwm_wr_ls_crc_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
			if (a_rd[7] == a_wr[0])
				result = TRUE;
	}

	/* write page3 */
	if (result) {
		result = FALSE;
		i = 0;
		a_wr[i++] = 0x0D;
		if (psw != 0xFF)
			a_wr[i++] = psw;
		a_wr[i++] = p_b9wk->page3[3];
		a_wr[i++] = p_b9wk->page3[2];
		a_wr[i++] = p_b9wk->page3[1];
		a_wr[i++] = p_b9wk->page3[0];
		if (tms37145_pwm_wr_ls_crc_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
			result = TRUE;		// don't check a_rd[7] because OTP tag return a_rd[7] is 0x0F
	}

	/* write page4 */
	if (result) {
		result = FALSE;
		i = 0;
		a_wr[i++] = 0x11;
		if (psw != 0xFF)
			a_wr[i++] = psw;
		a_wr[i++] = p_b9wk->page4[4];
		a_wr[i++] = p_b9wk->page4[3];
		a_wr[i++] = p_b9wk->page4[2];
		a_wr[i++] = p_b9wk->page4[1];
		a_wr[i++] = p_b9wk->page4[0];
		if (tms37145_pwm_wr_ls_crc_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
			if (a_rd[7] == a_wr[0])
				result = TRUE;
	}

	if (lock_flag) {
		if (result && (p_b9wk->page1[1] == e_page_lock))
			result = tms37145_lock_page(1, psw);
		
		if (result && (p_b9wk->page2[1] == e_page_lock))
			result = tms37145_lock_page(2, psw);
		
		if (result && (p_b9wk->page3[4] == e_page_lock))
			result = tms37145_lock_page(3, psw);
		
		if (result && (p_b9wk->page4[5] == e_page_lock))
			result = tms37145_lock_page(4, psw);
	}
		
	return result;
}
/*
void tms37145_get_data_init(void)
{
	lcd_clear_screen();
	lcd_print_str(0, 0, "GetData63 V1.1", 14);

	//led_on();
	//delay_us(500);
//	led_off();
}
*/
uint8_t tms37145_get_data(void)
{
	uint8_t result, i_fail=0, a_rd[12];
	const uint8_t a_wr = 0x04;

	enum {
		s_get_data,
		s_ok, s_wait_out, s_exit
	} state;

	result = FALSE;
	state = s_get_data;
	while (state != s_exit) {
		switch (state) {
			case s_get_data:
				if (!tms37145_pwm_wr_ls_rd(1, &a_wr, TMS37145_T_PWM_READ_RECH, a_rd)) {
					state = s_exit;
				}
				else if (tms37145_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8))) {
					state = s_exit;
				}
				else
					state = s_ok;
				break;
			case s_ok:
			//	led_on();
		//		uart_send_str(&a_rd[1], 6);
				lcd_print_hex(30, 40,200,24,24, &a_rd[1], 6);

				i_fail = 0;
				state = s_wait_out;
				break;
			case s_wait_out:
				if (tms37145_pwm_wr_ls_rd(1, &a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
					i_fail = 0;
				else
					i_fail++;

				if (i_fail == 2) {
//					led_off();
					lcd_print_blank(1);
					state = s_exit;
				}
				break;
			default :
				state = s_exit;
				break;
		}
	}

	return result;
}
/*
void tms37145_fast_read_init(void)
{
	lcd_clear_screen();
	lcd_print_str(0, 0, "FastRead4D V1.1", 15);

//	led_on();
//	delay_us(500);
//	led_off();
}
*/
uint8_t tms37145_fast_read(void)
{
	uint8_t result, i_fail=0, a_rd[12];
	const uint8_t a_wr = 0x04;

	enum {
		s_get_data,
		s_ok, s_wait_out, s_exit
	} state;

	result = FALSE;
	state = s_get_data;
	while (state != s_exit) {
		switch (state) {
			case s_get_data:
				if (!tms37145_pwm_wr_ls_rd(1, &a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
					state = s_exit;
				else if (tms37145_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
					state = s_exit;
				else
					state = s_ok;
				break;
			case s_ok:
//				led_on();
//				uart_send_str(&a_rd[1], 6);
				lcd_print_hex(30, 40,200,24,24, &a_rd[1], 6);

				i_fail = 0;
				state = s_wait_out;
				break;
			case s_wait_out:
				if (tms37145_pwm_wr_ls_rd(1, &a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
					i_fail = 0;
				else
					i_fail++;

				if (i_fail == 2) {
					//led_off();
					lcd_print_blank(1);
					state = s_exit;
				}
				break;
			default :
				state = s_exit;
				break;
		}
	}

	return result;
}
/*
void tag64_fast_read_init(void)
{
	lcd_clear_screen();
	lcd_print_str(0, 0, "FastRead64 V1.0", 15);

//	led_on();
//	delay_us(500);
//	led_off();
}
*/
uint8_t tag64_fast_read(void)
{
	uint8_t result, i_fail=0, a_rd[11];
	const uint8_t a_wr = 0x04;

	enum {
		s_get_data,
		s_ok, s_wait_out, s_exit
	} state;

	result = FALSE;
	state = s_get_data;
	while (state != s_exit) {
		switch (state) {
			case s_get_data:
				if (!tms37145_pwm_wr_ls_rd(1, &a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
					state = s_exit;
				else if (tms37145_calc_crc16(&a_rd[1], 5) != (a_rd[6] | ((uint16_t)a_rd[7]<<8)))
					state = s_exit;
				else
					state = s_ok;
				break;
			case s_ok:
//				led_on();
//				uart_send_str(&a_rd[1], 4);
				lcd_print_hex(30, 40,200,24,24, &a_rd[1], 4);
				result = TRUE;

				i_fail = 0;
				state = s_wait_out;
				break;
			case s_wait_out:
				if (tms37145_pwm_wr_ls_rd(1, &a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
					i_fail = 0;
				else
					i_fail++;

				if (i_fail >= 2) {
//					led_off();
					lcd_print_blank(1);
					state = s_exit;
				}
				break;
			default :
				state = s_exit;
				break;
		}
	}

	return result;
}
/*
void ts32_init_tag_init(void)
{
	lcd_clear_screen();
	lcd_print_str(0, 0, "TS32Init V1.1", 13);

//	led_on();
//	delay_us(500);
	//led_off();
}
*/
uint8_t ts32_init_tag(void)
{
	uint8_t result, i_fail, a_wr[4], a_rd[12];

	enum {
		s_check_cfg,
		s_set_all, s_wr_page6, s_wr_page5,
		s_ok, s_exit
	} state;

	result = FALSE;
	state = s_check_cfg;
	while (state != s_exit) {
		switch (state) {
			case s_check_cfg:
				a_wr[0] = 0x1C;
				if (!tms37145_pwm_wr_ls_rd(1, a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
					state = s_set_all;
				else if (a_rd[6] == 0xFD)
					state = s_ok;
				else
					state = s_set_all;
				break;

			case s_set_all:
				a_wr[0] = 0x1D;
				a_wr[1] = 0x00;
				tms37145_pwm_wr_ls_crc_rd(2, a_wr, TMS37145_T_PROG, a_rd);
				state = s_wr_page6;
				break;
			case s_wr_page6:
				a_wr[0] = 0x19;
				a_wr[1] = 0x00;
				a_wr[2] = 0xF8;	// V1.1 0xF8 V1.0 0x30
				tms37145_pwm_wr_ls_crc_rd(3, a_wr, TMS37145_T_PROG, a_rd);
				state = s_wr_page5;
				break;
			case s_wr_page5:
				a_wr[0] = 0x15;
				a_wr[1] = 0x00;
				a_wr[2] = 0xFD;
				tms37145_pwm_wr_ls_crc_rd(3, a_wr, TMS37145_T_PROG, a_rd);
				state = s_check_cfg;
				break;

			case s_ok:
//				led_on();
		//		uart_send_str(&a_rd[1], 7);
				lcd_print_hex(30, 40,200,24,24, &a_rd[1], 7);

				i_fail = 0;
				a_wr[0] = 0x1C;
				while (i_fail < 2) {
					if (tms37145_pwm_wr_ls_rd(1, a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
						i_fail = 0;
					else
						i_fail++;
				}
//				led_off();
				lcd_print_blank(1);
				result = TRUE;
				state = s_exit;
				break;
			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}

	return result;
}

uint8_t ts8x_test_read(uint8_t page, uint8_t *p_rd)
{
		uint8_t a_rd[11];
	uint8_t cmd = (0x80 | (page<<2));
	uint8_t result = FALSE;
	if (page > 31)
		return FALSE;

	if (tms37145_pwm_wr_ls_crc_rd(1, &cmd, TMS37145_T_PWM_READ_RECH, a_rd)) {
		copy_bytes(p_rd, &a_rd[3], 4);
		result = TRUE;
	}

	return result;
}

uint8_t ts8x_test_write(uint8_t page, const uint8_t *p_wr)
{
		uint8_t a_wr[5], a_rd[11];
	uint8_t result = FALSE;
	a_wr[0] = (0x81 | (page<<2));
	if (page > 31)
		return FALSE;


	copy_bytes(&a_wr[1], p_wr, 4);
	if (!tms37145_pwm_wr_ls_crc_rd(5, a_wr, TMS37145_T_PROG, a_rd))
		;
	/* compare return data and decide write result */
	else if (!equal_bytes(&a_wr[1], &a_rd[3], 4))
		;
	else
		result = TRUE;

	return result;
}

uint8_t ts80_unlock_all(void)
{
	uint8_t a_wr[5], a_rd[10];

	a_wr[0] = 0xF5;
	a_wr[1] = 0x00;
	a_wr[2] = 0x00;
	a_wr[3] = 0x00;
	a_wr[4] = 0x00;
	if (tms37145_pwm_wr_ls_crc_rd(5, a_wr, TMS37145_T_PROG, a_rd))
		if ((a_rd[3] == 0) && (a_rd[4] == 0) && (a_rd[5] == 0) 
		&& (a_rd[6] == 0) && (a_rd[7] == 0x74))
			return TRUE;

	return FALSE;
}

#define TAG63_ID_SAVE_INTERVAL		(0x40)
static uint16_t uid_i_tag63;
void ts8x_init_tag_init(void)
{
	uint8_t i, a_addr[2]={0};
	
//eep_rd_data(EEP_ADDR_TAG63_ID, a_addr, 2);
	uid_i_tag63 = ((uint16_t)a_addr[0]<<8) | ((uint16_t)a_addr[1]);
	uid_i_tag63 += TAG63_ID_SAVE_INTERVAL;			// jump to next segment

	i = 10;
	if (uid_i_tag63 >= TAG63_ID_MAX_NUM) {
		uid_i_tag63 = 0;
		a_addr[0] = 0;
		a_addr[1] = 0;
	//	for (; i > 0; i--)
		//	if (eep_wr_data(EEP_ADDR_TAG63_ID, a_addr, 2))
			//	break;
	}
  LCD_Clear(BACK_COLOR);
	//lcd_clear_screen();
	if (i > 0) {
		#if 1
		debug_printf("\r\nTS86Init63 V1.5\r\n");
		//lcd_print_str(0, 0, "TS86Init63 V1.5", 15);
		#else
		//lcd_print_str(0, 0, "TS85Init4D V1.0", 15);
		debug_printf("\r\nTS85Init4D V1.0\r\n");
		#endif
		//led_on();
	//	delay_us(500*1000);
	//	led_off();
	}
	else {
		//lcd_print_str(0, 0, "ATmega128 error", 15);
		debug_printf("\r\nATmega128 error\r\n");
		while(1);
	}
}
#if _INIT_TS_TAG
uint8_t ts86_init_tag63(void)
{
	static const uint8_t a_tp29[4] = {0x00, 0x00, 0x00, 0x00};
	static const uint8_t a_tp30[4] = {0x00, 0x46, 0x00, 0x00};
	static const uint8_t a_tp31[4] = {0x2B, 0xF8, 0x00, 0x00};
	
	uint8_t result, i, i_fail, a_wr[8], a_rd[11];
	uint8_t a_id[6];
	enum {
		s_wr_tp30_31_29, s_wr_tp0to2, s_wr_page4,
		s_ok, s_exit
	} state;

	result = FALSE;
	state = s_wr_tp30_31_29;
	while (state != s_exit) {
		switch (state) {
			case s_wr_tp30_31_29:
				if (!ts8x_test_write(30, a_tp30))
					state = s_exit;
				else if (!ts8x_test_write(31, a_tp31))
					state = s_exit;
				else if (!ts8x_test_write(29, a_tp29))
					state = s_exit;
				else
					state = s_wr_tp0to2;
				break;
			case s_wr_tp0to2:
				uid_i_tag63++;
				if (uid_i_tag63 >= TAG63_ID_MAX_NUM)
					uid_i_tag63 = 0;
				/* reduce write eeprom times */
				if ((uid_i_tag63 % TAG63_ID_SAVE_INTERVAL) == 0) {
					a_id[0] = (uid_i_tag63 >> 8);
					a_id[1] = uid_i_tag63;
				//	if (!eep_wr_data(EEP_ADDR_TAG63_ID, a_id, 2))
						//eep_wr_data(EEP_ADDR_TAG63_ID, a_id, 2);
				}

				for (i = 0; i < 6; i++)
					a_id[i] = a_tag63_uid1[uid_i_tag63][i];
				while ((a_id[0] != 0x03) || (a_id[1] != 0x80) || (a_id[2] != 0x01)) {
					debug_printf("\r\nID Error\r\n");
				//	led_flicker();
					delay_us(2000*1000);
				}
				
				a_wr[0] = a_id[0];
				a_wr[1] = 0x00;
				a_wr[2] = 0x00;
				a_wr[3] = 0x00;
				a_wr[4] = a_id[1];
				a_wr[5] = 0x00;
				a_wr[6] = 0x00;
				a_wr[7] = 0x00;
				if (!ts8x_test_write(0, &a_wr[0]))
					state = s_exit;
				else if (!ts8x_test_write(1, &a_wr[4]))
					state = s_exit;
				else if (!ts8x_test_write(2, &a_id[2]))
					state = s_exit;
				else
					state = s_wr_page4;
				break;

			case s_wr_page4:
				a_wr[0] = 0x11;
				a_wr[1] = a_id[0];
				a_wr[2] = 0x00;
				a_wr[3] = 0x00;
				a_wr[4] = 0x00;
				a_wr[5] = 0x00;
				a_wr[6] = 0x00;
				state = s_exit;
				if (tms37145_pwm_wr_ls_crc_rd(7, &a_wr[0], TMS37145_T_PROG, a_rd))
					if (equal_bytes(&a_rd[1], &a_id[3], 3))
						state = s_ok;
				break;

			case s_ok:
				//led_on();
				lcd_print_hex(30, 40,200,24,24, a_id, 6);
				lcd_print_hex(30, 40,200,24,24, a_tp31, 2);

				i_fail = 0;
				a_wr[0] = 0x04;
				while (i_fail < 2) {
					if (tms37145_pwm_wr_ls_rd(1, a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
						i_fail = 0;
					else
						i_fail++;
				}
			//	led_off();
				lcd_print_blank(1);
				result = TRUE;
				state = s_exit;
				break;

			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}

	return result;
}
uint8_t ts85_init_tag4d(void)
{
	static const uint8_t a_tp29[4] = {0x00, 0x00, 0x00, 0x20};
	static const uint8_t a_tp30[4] = {0xF0, 0x66, 0x00, 0x00};
	static const uint8_t a_tp31[4] = {0x2B, 0xF8, 0x00, 0x00};
	
	uint8_t result, i, i_fail, a_wr[8], a_rd[11];
	uint8_t a_id[6];
	enum {
		s_wr_tp30_31_29, s_wr_tp0to2, s_wr_page4,
		s_ok, s_exit
	} state;

	result = FALSE;
	state = s_wr_tp30_31_29;
	while (state != s_exit) {
		switch (state) {
			case s_wr_tp30_31_29:
				if (!ts8x_test_write(30, a_tp30))
					state = s_exit;
				else if (!ts8x_test_write(31, a_tp31))
					state = s_exit;
				else if (!ts8x_test_write(29, a_tp29))
					state = s_exit;
				else
					state = s_wr_tp0to2;
				break;

			case s_wr_tp0to2:
				uid_i_tag63++;
				if (uid_i_tag63 >= TAG63_ID_MAX_NUM)
					uid_i_tag63 = 0;
				/* reduce write eeprom times */
				if ((uid_i_tag63 % TAG63_ID_SAVE_INTERVAL) == 0) {
					a_id[0] = (uid_i_tag63 >> 8);
					a_id[1] = uid_i_tag63;
					//if (!eep_wr_data(EEP_ADDR_TAG63_ID, a_id, 2))
					//	if (!eep_wr_data(EEP_ADDR_TAG63_ID, a_id, 2))
					//		eep_wr_data(EEP_ADDR_TAG63_ID, a_id, 2);
				}

				for (i = 0; i < 6; i++)
					a_id[i] = a_tag63_uid1[uid_i_tag63][i];
				while ((a_id[0] != 0x03) || (a_id[1] != 0x80) || (a_id[2] != 0x01)) {
					debug_printf("\r\nID Error\r\n");
				//	led_flicker();
					delay_us(2000*1000);
				}
				
				a_wr[0] = a_id[0];
				a_wr[1] = 0x00;
				a_wr[2] = 0x00;
				a_wr[3] = 0x00;
				a_wr[4] = a_id[1];
				a_wr[5] = 0x00;
				a_wr[6] = 0x00;
				a_wr[7] = 0x00;
				if (!ts8x_test_write(0, &a_wr[0]))
					state = s_exit;
				else if (!ts8x_test_write(1, &a_wr[4]))
					state = s_exit;
				else if (!ts8x_test_write(2, &a_id[2]))
					state = s_exit;
				else
					state = s_wr_page4;
				break;

			case s_wr_page4:
				a_wr[0] = 0x11;
				a_wr[1] = a_id[0];
				a_wr[2] = 0x00;
				a_wr[3] = 0x00;
				a_wr[4] = 0x00;
				a_wr[5] = 0x00;
				a_wr[6] = 0x00;
				state = s_exit;
				if (tms37145_pwm_wr_ls_crc_rd(7, &a_wr[0], TMS37145_T_PROG, a_rd))
					if (equal_bytes(&a_rd[1], &a_id[3], 3))
						state = s_ok;
				break;

			case s_ok:
			//	led_on();
				lcd_print_hex(30, 40,200,24,24,a_id, 6);
				lcd_print_hex(30, 40,200,24,24, a_tp31, 2);

				i_fail = 0;
				a_wr[0] = 0x04;
				while (i_fail < 2) {
					if (tms37145_pwm_wr_ls_rd(1, a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
						i_fail = 0;
					else
						i_fail++;
				}
			//	led_off();
				lcd_print_blank(1);
				result = TRUE;
				state = s_exit;
				break;
			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}

	return result;
}
#endif
#define TS0DNEW_MODE_60		(0x03)
#define TS0DNEW_MODE_64		(0x07)
#define TS0DNEW_MODE_4C		(0x08)

/********************************************************
* taisu 0D new tag, TangYouWei design
********************************************************/
uint8_t ts0dnew_test_read(uint8_t page, uint8_t *p_rd)
{
		uint8_t a_rd[11];
	uint8_t cmd = (0x82 | (page<<2));
	uint8_t result = FALSE;
	if (page > 31)
		return FALSE;

	if (tms37145_pwm_wr_ls_crc_rd(1, &cmd, TMS37145_T_PWM_READ_RECH, a_rd)) {
		copy_bytes(p_rd, &a_rd[3], 4);
		result = TRUE;
	}

	return result;
}

uint8_t ts0dnew_test_write(uint8_t page, const uint8_t *p_wr)
{
		uint8_t a_wr[5], a_rd[11];
		uint8_t result = FALSE;
	if (page > 31)
		return FALSE;


	a_wr[0] = (0x83 | (page<<2));
	copy_bytes(&a_wr[1], p_wr, 4);
	if (!tms37145_pwm_wr_ls_crc_rd(5, a_wr, TMS37145_T_PROG, a_rd))
		;
	/* compare return data and decide write result */
	else if (!equal_bytes(&a_wr[1], &a_rd[3], 4))
		;
	else
		result = TRUE;

	return result;
}

uint8_t ts0dnew_check_mode(uint8_t mode)
{
	uint8_t result, a_rd[4];

	result= FALSE;
	if (!ts0dnew_test_read(28, a_rd))
		;
	else if ((a_rd[1] & 0x0C) != (mode & 0x0C))
		;
	else
		result = TRUE;

	return result;
}

uint8_t ts0dnew_copy_b9wk(const tms37145_tag_t *p_b9wk)
{
		uint8_t a_wr[5];
	/* V2.25B, leader ChengZQ popularize TS0D(64) */
	if (p_b9wk->read_len == 8)
		if (!ts0dnew_check_mode(TS0DNEW_MODE_64))
			return FALSE;

	a_wr[0] = 0x00;
	a_wr[1] = TS0DNEW_MODE_60;
	a_wr[2] = 0x00;
	a_wr[3] = 0x00;

	ts0dnew_test_write(28, a_wr);		// first could not return data
	if (!ts0dnew_test_write(28, a_wr))	// change to 60 mode
		return FALSE;

	a_wr[1] = 0x00;
	if (!ts0dnew_test_write(29, a_wr))	//unlock all page
		return FALSE;

	if (!b9wk_set_tag(p_b9wk, TRUE))
		return FALSE;

	if (p_b9wk->read_len == 8) {
		a_wr[1] = TS0DNEW_MODE_64;
		if (!ts0dnew_test_write(28, a_wr)) // change to 64 mode
			return FALSE;
	}

	return TRUE;
}

uint8_t ts0dnew_copy_r9wk(const r9wk_tag_t *p_r9wk)
{
	r9wk_tag_t tag;
	uint8_t result, a_wr[5];
	a_wr[0] = 0x00;
	a_wr[1] = TS0DNEW_MODE_60;
	a_wr[2] = 0x00;
	a_wr[3] = 0x00;

	result = FALSE;
	ts0dnew_test_write(28, a_wr);	// first could not return data
	if (!ts0dnew_test_write(28, a_wr))
		;
	else if (!ts0dnew_test_write(9, &(p_r9wk->a_id[0])))
		;
	else if (!ts0dnew_test_write(10, &(p_r9wk->a_id[4])))
		;
	else if (!ts0dnew_test_write(11, &(p_r9wk->a_id[8])))
		;
	else if (!ts0dnew_test_write(12, &(p_r9wk->end[1])))
		;
	else {
		a_wr[1] = TS0DNEW_MODE_4C;
		result = ts0dnew_test_write(28, a_wr);;
	}

	if (!result)
		;
	else if (!r9wk_find_tag(&tag))
		result = FALSE;
	else if (!equal_bytes(p_r9wk->a_id, tag.a_id, 10))
		result = FALSE;

	return result;
}

void ts0d_init_tag63_1_init(void)
{
	uint8_t i, a_addr[2]={0};
	
//	eep_rd_data(EEP_ADDR_TAG63_ID, a_addr, 2);
	uid_i_tag63 = ((uint16_t)a_addr[0]<<8) | ((uint16_t)a_addr[1]);
	uid_i_tag63 += TAG63_ID_SAVE_INTERVAL;			// jump to next segment

	i = 10;
	if (uid_i_tag63 >= TAG63_ID_MAX_NUM) {
		uid_i_tag63 = 0;
		a_addr[0] = 0;
		a_addr[1] = 0;
		//for (; i > 0; i--)
			//if (eep_wr_data(EEP_ADDR_TAG63_ID, a_addr, 2))
			//	break;
	}

	//lcd_clear_screen();
	if (i > 0) {
		debug_printf("\r\nTS0DIn63-1 V1.1\r\n");
		//lcd_print_str(0, 0, "TS0DIn63-1 V1.1", 15);
	//	led_on();
//		delay_us(500);
	//	led_off();
	}
	else {
		//lcd_print_str(0, 0, "ATmega128 error", 15);
    debug_printf("\r\nATmega128 error\r\n");
		while(1);
	}
}
#if _INIT_TS_TAG
uint8_t ts0d_init_tag63_1(void)
{
	static const uint8_t a_tp29[4] = {0x20, 0x00, 0x00, 0x00};		//lock page3
	static const uint8_t a_tp30_40[4] = {0x00, 0x46, 0x00, 0x00};
	static const uint8_t a_tp30_80[4] = {0x08, 0x46, 0x00, 0x00};
	static const uint8_t a_tp31[4] = {0x2B, 0xF8, 0x00, 0x00};		// DST80 algorithm correct

	uint8_t result, i, i_fail, a_wr[13], a_rd[11];
	uint8_t a_id[6];
	enum {
		s_wr_tp30_31_29, s_wr_tp0to2, s_wr_page4_7, s_wr_tp30, 
		s_ok, s_exit
	} state;

	result = FALSE;
	state = s_wr_tp30_31_29;
	while (state != s_exit) {
		switch (state) {
			case s_wr_tp30_31_29:
				if (!ts0dnew_test_write(30, a_tp30_80))
					state = s_exit;
				else if (!ts0dnew_test_write(31, a_tp31))
					state = s_exit;
				else if (!ts0dnew_test_write(29, a_tp29))
					state = s_exit;
				else
					state = s_wr_tp0to2;
				break;

			case s_wr_tp0to2:
				uid_i_tag63++;
				if (uid_i_tag63 >= TAG63_ID_MAX_NUM)
					uid_i_tag63 = 0;
				/* reduce write eeprom times */
				if ((uid_i_tag63 % TAG63_ID_SAVE_INTERVAL) == 0) {
					a_id[0] = (uid_i_tag63 >> 8);
					a_id[1] = uid_i_tag63;
				//	if (!eep_wr_data(EEP_ADDR_TAG63_ID, a_id, 2))
					//	if (!eep_wr_data(EEP_ADDR_TAG63_ID, a_id, 2))
						//	eep_wr_data(EEP_ADDR_TAG63_ID, a_id, 2);
				}

				for (i = 0; i < 6; i++)
					a_id[i] = a_tag63_uid1[uid_i_tag63][i];
				while ((a_id[0] != 0x03) || (a_id[1] != 0x80) || (a_id[2] != 0x01)) {
						debug_printf("\r\nID Error\r\n");
					//lcd_print_str(0, 1, "ID Error", 8);
			//		led_flicker();
					delay_us(2000*1000);
				}
				
				a_wr[0] = a_id[0];
				a_wr[1] = 0x00;
				a_wr[2] = 0x00;
				a_wr[3] = 0x00;
				a_wr[4] = a_id[1];
				a_wr[5] = 0x00;
				a_wr[6] = 0x00;
				a_wr[7] = 0x00;
				if (!ts0dnew_test_write(0, &a_wr[0]))
					state = s_exit;
				else if (!ts0dnew_test_write(1, &a_wr[4]))
					state = s_exit;
				else if (!ts0dnew_test_write(2, &a_id[2]))
					state = s_exit;
				else
					state = s_wr_page4_7;
				break;

			case s_wr_page4_7:
				a_wr[0] = 4*4 + 1;
				a_wr[1] = a_id[0];
				a_wr[2] = 0x00;
				a_wr[3] = 0x00;
				a_wr[4] = 0x00;
				a_wr[5] = 0x00;
				a_wr[6] = 0x00;
				a_wr[7] = 0x00;
				a_wr[8] = 0x00;
				a_wr[9] = 0x00;
				a_wr[10] = 0x00;
				a_wr[11] = 0x00;
				
				state = s_exit;
				if (!tms37145_pwm_wr_ls_crc_rd(12, &a_wr[0], TMS37145_T_PROG, a_rd))
					;
				else if (!equal_bytes(&a_rd[1], &a_id[3], 3))
					;
				else
					a_wr[0] = 4*7 + 1;

				if (a_wr[0] != 4*7 + 1)
					;
				if (!tms37145_pwm_wr_ls_crc_rd(12, &a_wr[0], TMS37145_T_PROG, a_rd))
					;
				else if ((a_rd[1] != a_id[1]) || (a_rd[7] != a_wr[0]))
					;
				else
					state = s_wr_tp30;

				break;

			case s_wr_tp30:
				if (!ts0dnew_test_write(30, a_tp30_40))
					state = s_exit;
				else
					state = s_ok;
				break;

			case s_ok:
			//	led_on();
				lcd_print_hex(30, 40,200,24,24, a_id, 6);
				lcd_print_hex(30, 40,200,24,24, a_tp31, 2);

				i_fail = 0;
				a_wr[0] = 0x04;
				while (i_fail < 2) {
					if (tms37145_pwm_wr_ls_rd(1, a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
						i_fail = 0;
					else
						i_fail++;
				}
			//	led_off();
				lcd_print_blank(1);
				result = TRUE;
				state = s_exit;
				break;

			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}

	return result;
}
#endif
/*void ts0d_init_tag63_2_init(void)
{
	lcd_clear_screen();
	lcd_print_str(0, 0, "TS0DIn63-2 V1.1", 15);
	//led_on();
//	delay_us(500);
//	led_off();
}*/

uint8_t ts0d_init_tag63_2(void)
{
	//static const uint8_t a_jma_id[4] = {0x3E, 0x82, 0xA1, 0x3A};	// LSB-->MSB
	
	uint8_t result, i, i_fail, psw=0, a_wr[8], a_rd[11];
	enum {
		s_find_tag, s_wr_page8_15, s_wr_page18,
		s_ok, s_exit
	} state;

	result = FALSE;
	state = s_find_tag;
	while (state != s_exit) {
		switch (state) {
			case s_find_tag:
				a_wr[0] = 0x04;
				if (tms37145_pwm_wr_ls_rd(1, a_wr, TMS37145_T_PWM_READ_RECH, a_rd)) {
					psw = a_rd[1];
					state = s_wr_page8_15;
				}
				else
					state = s_exit;
				break;
			/*
			case s_wr_tp27:
				if (ts0dnew_test_write(27, a_jma_id))
					state = s_wr_page8_15;
				else
					state = s_exit;
				break;
			*/
			case s_wr_page8_15:
				a_wr[1] = psw;
				a_wr[2] = 0x00;
				a_wr[3] = 0x00;
				a_wr[4] = 0x00;
				a_wr[5] = 0x00;
				a_wr[6] = 0x00;
				
				for (i = 8; i < 16; i++) {
					a_wr[0] = i*4 + 1;
					if (!tms37145_pwm_wr_ls_crc_rd(7, &a_wr[0], TMS37145_T_PROG, a_rd))
						break;
					else if (!equal_bytes(&a_wr[2], &a_rd[2], 5))
						break;
				}
				state = (i < 16) ? s_exit : s_wr_page18;
				break;

			case s_wr_page18:
				a_wr[0] = 18*4 + 1;
				a_wr[1] = 0x00;
				if (!tms37145_pwm_wr_ls_crc_rd(2, &a_wr[0], TMS37145_T_PROG, a_rd))
					state = s_exit;
				else if (a_wr[1] != a_rd[1])
					state = s_exit;
				else
					state = s_ok;
				break;

			case s_ok:
			//	led_on();
				lcd_print_hex(30, 40,200,24,24, &a_rd[1], 1);

				a_wr[0] = 0x04;
				i_fail = 0;
				while (i_fail < 2) {
					if (tms37145_pwm_wr_ls_rd(1, a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
						i_fail = 0;
					else
						i_fail++;
				}
			//	led_off();
				lcd_print_blank(1);
				result = TRUE;
				state = s_exit;
				break;

			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}

	return result;
}

void ts0d_init_tag63_init(void)
{
	uint8_t i, a_addr[2]={0};
	
//	eep_rd_data(EEP_ADDR_TAG63_ID, a_addr, 2);
	uid_i_tag63 = ((uint16_t)a_addr[0]<<8) | ((uint16_t)a_addr[1]);
	uid_i_tag63 += TAG63_ID_SAVE_INTERVAL;			// jump to next segment

	i = 10;
	if (uid_i_tag63 >= TAG63_ID_MAX_NUM) {
		uid_i_tag63 = 0;
		a_addr[0] = 0;
		a_addr[1] = 0;
	//	for (; i > 0; i--)
			//if (eep_wr_data(EEP_ADDR_TAG63_ID, a_addr, 2))
			//	break;
	}

	//lcd_clear_screen();
	if (i > 0) {
		debug_printf("\r\nTS0DInit63 V2.0\r\n");
	//	led_on();
//		delay_us(500);
	//	led_off();
	}
	else {
		debug_printf("\r\nATmega128 error\r\n");
		//lcd_print_str(0, 0, "ATmega128 error", 15);
		while(1);
	}
}
#if _INIT_TS_TAG
uint8_t ts0d_init_tag63(void)
{
	static const uint8_t a_tp29[4] = {0x20, 0x00, 0x00, 0x00};		//lock page3
	static const uint8_t a_tp30_40[4] = {0x00, 0x46, 0x00, 0x00};
	static const uint8_t a_tp30_80[4] = {0x08, 0x46, 0x00, 0x00};
	static const uint8_t a_tp31[4] = {0x2B, 0xF8, 0x00, 0x00};		// DST80 algorithm correct

	uint8_t result, i, i_fail, a_wr[13], a_rd[11];
	uint8_t a_id[6];
	enum {
		s_wr_tp30_31_29, s_wr_tp0to2, s_wr_page4_7, s_wr_tp30, 
		s_wr_page8_15, s_wr_page18, 
		s_ok, s_exit
	} state;

	result = FALSE;
	state = s_wr_tp30_31_29;
	while (state != s_exit) {
		switch (state) {
			case s_wr_tp30_31_29:
				if (!ts0dnew_test_write(30, a_tp30_80))
					state = s_exit;
				else if (!ts0dnew_test_write(31, a_tp31))
					state = s_exit;
				else if (!ts0dnew_test_write(29, a_tp29))
					state = s_exit;
				else
					state = s_wr_tp0to2;
				break;

			case s_wr_tp0to2:
				uid_i_tag63++;
				if (uid_i_tag63 >= TAG63_ID_MAX_NUM)
					uid_i_tag63 = 0;
				/* reduce write eeprom times */
				if ((uid_i_tag63 % TAG63_ID_SAVE_INTERVAL) == 0) {
					a_id[0] = (uid_i_tag63 >> 8);
					a_id[1] = uid_i_tag63;
				//	if (!eep_wr_data(EEP_ADDR_TAG63_ID, a_id, 2))
					//	if (!eep_wr_data(EEP_ADDR_TAG63_ID, a_id, 2))
						//	eep_wr_data(EEP_ADDR_TAG63_ID, a_id, 2);
				}

				for (i = 0; i < 6; i++)
					a_id[i] = a_tag63_uid1[uid_i_tag63][i];
				while ((a_id[0] != 0x03) || (a_id[1] != 0x80) || (a_id[2] != 0x01)) {
					debug_printf("\r\nID Error\r\n");
					//lcd_print_str(0, 1, "ID Error", 8);
			//		led_flicker();
					delay_us(2000*1000);
				}
				
				a_wr[0] = a_id[0];
				a_wr[1] = 0x00;
				a_wr[2] = 0x00;
				a_wr[3] = 0x00;
				a_wr[4] = a_id[1];
				a_wr[5] = 0x00;
				a_wr[6] = 0x00;
				a_wr[7] = 0x00;
				if (!ts0dnew_test_write(0, &a_wr[0]))
					state = s_exit;
				else if (!ts0dnew_test_write(1, &a_wr[4]))
					state = s_exit;
				else if (!ts0dnew_test_write(2, &a_id[2]))
					state = s_exit;
				else
					state = s_wr_page4_7;
				break;

			case s_wr_page4_7:
				a_wr[0] = 4*4 + 1;
				a_wr[1] = a_id[0];
				a_wr[2] = 0x00;
				a_wr[3] = 0x00;
				a_wr[4] = 0x00;
				a_wr[5] = 0x00;
				a_wr[6] = 0x00;
				a_wr[7] = 0x00;
				a_wr[8] = 0x00;
				a_wr[9] = 0x00;
				a_wr[10] = 0x00;
				a_wr[11] = 0x00;
				
				state = s_exit;
				if (!tms37145_pwm_wr_ls_crc_rd(12, &a_wr[0], TMS37145_T_PROG, a_rd))
					;
				else if (!equal_bytes(&a_rd[1], &a_id[3], 3))
					;
				else
					a_wr[0] = 4*7 + 1;

				if (a_wr[0] != 4*7 + 1)
					;
				if (!tms37145_pwm_wr_ls_crc_rd(12, &a_wr[0], TMS37145_T_PROG, a_rd))
					;
				else if ((a_rd[1] != a_id[1]) || (a_rd[7] != a_wr[0]))
					;
				else
					state = s_wr_tp30;

				break;

			case s_wr_tp30:
				if (!ts0dnew_test_write(30, a_tp30_40))
					state = s_exit;
				else
					state = s_wr_page8_15;
				break;

			case s_wr_page8_15:
				a_wr[1] = a_id[0];
				a_wr[2] = 0x00;
				a_wr[3] = 0x00;
				a_wr[4] = 0x00;
				a_wr[5] = 0x00;
				a_wr[6] = 0x00;
				
				for (i = 8; i < 16; i++) {
					a_wr[0] = i*4 + 1;
					if (!tms37145_pwm_wr_ls_crc_rd(7, &a_wr[0], TMS37145_T_PROG, a_rd))
						break;
					else if (!equal_bytes(&a_wr[2], &a_rd[2], 5))
						break;
				}
				state = (i < 16) ? s_exit : s_wr_page18;
				break;

			case s_wr_page18:
				a_wr[0] = 18*4 + 1;
				a_wr[1] = 0x00;
				if (!tms37145_pwm_wr_ls_crc_rd(2, &a_wr[0], TMS37145_T_PROG, a_rd))
					state = s_exit;
				else if (a_wr[1] != a_rd[1])
					state = s_exit;
				else
					state = s_ok;
				break;

			case s_ok:
			//	led_on();
				lcd_print_hex(30, 40,200,24,24, a_id, 6);
				lcd_print_hex(30, 40,200,24,24, a_tp31, 2);

				i_fail = 0;
				a_wr[0] = 0x04;
				while (i_fail < 2) {
					if (tms37145_pwm_wr_ls_rd(1, a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
						i_fail = 0;
					else
						i_fail++;
				}
			//	led_off();
				lcd_print_blank(1);
				result = TRUE;
				state = s_exit;
				break;

			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}

	return result;
}
#endif
/*
void ts0d_init_tag4d_init(void)
{
	lcd_clear_screen();
	lcd_print_str(0, 0, "TS0DIni 4D V1.1", 15);
//	led_on();
	//delay_us(500);
	//led_off();
}
*/
uint8_t ts0d_init_tag4d(void)
{
	static const uint8_t a_tp30[4] = {0x00, 0x46, 0x00, 0x00};
	static const uint8_t a_tp31[4] = {0x54, 0xF8, 0x00, 0x00};		// DST80 algorithm error
	static const uint8_t a_tp29[4] = {0x00, 0x00, 0x00, 0x20};		// lock page30
	static const uint8_t a_tp28[4] = {0x00, TS0DNEW_MODE_60, 0x00, 0x00};
	static const uint8_t a_jma_id[4] = {0x3E, 0x82, 0xA1, 0x3A};	// LSB-->MSB

	static const uint8_t a_id_67[6] = {0x52, 0x2C, 0x11, 0xE3, 0x19, 0xBD};
	//static const uint8_t a_key_67[5] = {0x1E, 0xCE, 0x80, 0x59, 0x3C};

	uint8_t result, i, psw=0;
	uint8_t a_wr[13], a_rd[11];
	enum {
		s_wr_tp30_31_29_28_27, s_get_psw, 
		s_wr_page1, s_wr_page2, s_wr_page3, s_wr_page4, 
		s_ok, s_exit
	} state;

	result = FALSE;
	state = s_wr_tp30_31_29_28_27;
	while (state != s_exit) {
		switch (state) {
			case s_wr_tp30_31_29_28_27:
				if (!ts0dnew_test_write(30, a_tp30))
					state = s_exit;
				else if (!ts0dnew_test_write(31, a_tp31))
					state = s_exit;
				else if (!ts0dnew_test_write(29, a_tp29))
					state = s_exit;
				else if (!ts0dnew_test_write(28, a_tp28))
					state = s_exit;
				else if (!ts0dnew_test_write(27, a_jma_id))
					state = s_exit;
				else
					state = s_get_psw;
				break;

			case s_get_psw:
				a_wr[0] = 0x04;
				if (!tms37145_pwm_wr_ls_rd(1, &a_wr[0], TMS37145_T_PWM_READ_RECH, a_rd))
					state = s_exit;
				else if (tms37145_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
					state = s_exit;
				else {
					psw = a_rd[1];
					state = s_wr_page1;
				}
				break;

			case s_wr_page1:
				i = 0;
				a_wr[i++] = 4*1 + 1;
				if (psw != 0xFF)
					a_wr[i++] = psw;
				a_wr[i++] = a_id_67[0];
				if (!tms37145_pwm_wr_ls_crc_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
					state = s_exit;
				else if (a_rd[1] != a_id_67[0])
					state = s_exit;
				else {
					psw = a_id_67[0];
					state = s_wr_page2;
				}
				break;

			case s_wr_page2:
				i = 0;
				a_wr[i++] = 4*2 + 1;
				if (psw != 0xFF)
					a_wr[i++] = psw;
				a_wr[i++] = a_id_67[1];
				if (!tms37145_pwm_wr_ls_crc_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
					state = s_exit;
				else if (a_rd[2] != a_id_67[1])
					state = s_exit;
				else
					state = s_wr_page3;
				break;

			case s_wr_page3:
				i = 0;
				a_wr[i++] = 4*3 + 1;
				if (psw != 0xFF)
					a_wr[i++] = psw;
				a_wr[i++] = a_id_67[2];
				a_wr[i++] = a_id_67[3];
				a_wr[i++] = a_id_67[4];
				a_wr[i++] = a_id_67[5];
				if (!tms37145_pwm_wr_ls_crc_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
					state = s_exit;
				else if (a_rd[3] != a_id_67[2])
					state = s_exit;
				else
					state = s_wr_page4;
				break;

			case s_wr_page4:
				i = 0;
				a_wr[i++] = 4*4 + 1;
				if (psw != 0xFF)
					a_wr[i++] = psw;
				a_wr[i++] = 0x00;
				a_wr[i++] = 0x00;
				a_wr[i++] = 0x00;
				a_wr[i++] = 0x00;
				a_wr[i++] = 0x00;
				if (!tms37145_pwm_wr_ls_crc_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
					state = s_exit;
				else if (a_rd[7] != a_wr[0])
					state = s_exit;
				else
					state = s_ok;
				break;

			case s_ok:
			//	led_on();
				lcd_print_hex(30, 40,200,24,24, a_id_67, 6);
				lcd_print_hex(30, 40,200,24,24, a_tp31, 2);

				i = 0;
				a_wr[0] = 0x04;
				while (i < 2) {
					if (tms37145_pwm_wr_ls_rd(1, a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
						i = 0;
					else
						i++;
				}
		//		led_off();
				lcd_print_blank(1);
				result = TRUE;
				state = s_exit;
				break;

			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}

	return result;
}
/*
void ts0d_init_tag64_init(void)
{
	lcd_clear_screen();
	lcd_print_str(0, 0, "TS0DIni 64 V1.1", 15);
	//led_on();
//	delay_us(500);
	//led_off();
}*/
#if _INIT_TS_TAG
uint8_t ts0d_init_tag64(void)
{
	static const uint8_t a_tp30[4] = {0x00, 0x46, 0x00, 0x00};
	static const uint8_t a_tp31[4] = {0x54, 0xF8, 0x00, 0x00};		// DST80 algorithm error
	static const uint8_t a_tp29[4] = {0x00, 0x00, 0x00, 0x20};		// lock page30
	static const uint8_t a_tp28[4] = {0x00, TS0DNEW_MODE_64, 0x00, 0x00};
	static const uint8_t a_jma_id[4] = {0x3E, 0x82, 0xA1, 0x3A};	// LSB-->MSB

	static uint8_t i_id;
	uint8_t result, i, psw=0;
	uint8_t a_wr[13], a_rd[11];
	enum {
		s_wr_tp30_31_29_28_27, s_get_psw, 
		s_wr_page1, s_wr_page2, s_wr_page3, s_wr_page4, 
		s_ok, s_exit
	} state;

	result = FALSE;
	state = s_wr_tp30_31_29_28_27;
	while (state != s_exit) {
		switch (state) {
			case s_wr_tp30_31_29_28_27:
				if (!ts0dnew_test_write(30, a_tp30))
					state = s_exit;
				else if (!ts0dnew_test_write(31, a_tp31))
					state = s_exit;
				else if (!ts0dnew_test_write(29, a_tp29))
					state = s_exit;
				else if (!ts0dnew_test_write(28, a_tp28))
					state = s_exit;
				else if (!ts0dnew_test_write(27, a_jma_id))
					state = s_exit;
				else
					state = s_get_psw;
				break;

			case s_get_psw:
				a_wr[0] = 0x04;
				if (!tms37145_pwm_wr_ls_rd(1, &a_wr[0], TMS37145_T_PWM_READ_RECH, a_rd))
					state = s_exit;
				else if (tms37145_calc_crc16(&a_rd[1], 5) != (a_rd[6] | ((uint16_t)a_rd[7]<<8)))
					state = s_exit;
				else {
					psw = a_rd[1];
					state = s_wr_page1;
				}
				break;

			case s_wr_page1:
				i_id++;
				if (i_id >= 3)
					i_id = 0;

				i = 0;
				a_wr[i++] = 4*1 + 1;
				if (psw != 0xFF)
					a_wr[i++] = psw;
				a_wr[i++] = a_tag64_uid1[i_id][0];
				if (!tms37145_pwm_wr_ls_crc_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
					state = s_exit;
				else if (a_rd[1] != a_tag64_uid1[i_id][0])
					state = s_exit;
				else {
					psw = a_tag64_uid1[i_id][0];
					state = s_wr_page2;
				}
				break;

			case s_wr_page2:
				i = 0;
				a_wr[i++] = 4*2 + 1;
				if (psw != 0xFF)
					a_wr[i++] = psw;
				a_wr[i++] = a_tag64_uid1[i_id][1];
				if (!tms37145_pwm_wr_ls_crc_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
					state = s_exit;
				else if (a_rd[1] != a_tag64_uid1[i_id][1])
					state = s_exit;
				else
					state = s_wr_page3;
				break;

			case s_wr_page3:
				i = 0;
				a_wr[i++] = 4*3 + 1;
				if (psw != 0xFF)
					a_wr[i++] = psw;
				a_wr[i++] = a_tag64_uid1[i_id][2];
				a_wr[i++] = a_tag64_uid1[i_id][3];
				a_wr[i++] = a_tag64_uid1[i_id][4];
				a_wr[i++] = a_tag64_uid1[i_id][5];
				if (!tms37145_pwm_wr_ls_crc_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
					state = s_exit;
				else if (a_rd[1] != a_tag64_uid1[i_id][2])
					state = s_exit;
				else
					state = s_wr_page4;
				break;

			case s_wr_page4:
				i = 0;
				a_wr[i++] = 4*4 + 1;
				if (psw != 0xFF)
					a_wr[i++] = psw;
				a_wr[i++] = 0x00;
				a_wr[i++] = 0x00;
				a_wr[i++] = 0x00;
				a_wr[i++] = 0x00;
				a_wr[i++] = 0x00;
				if (!tms37145_pwm_wr_ls_crc_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
					state = s_exit;
				else if (a_rd[5] != a_wr[0])
					state = s_exit;
				else
					state = s_ok;
				break;

			case s_ok:
				for (i = 0; i < 6; i++)
					a_wr[i] = a_tag64_uid1[i_id][i];
				//led_on();
				lcd_print_hex(30, 40,200,24,24, a_wr, 6);
				lcd_print_hex(30, 40,200,24,24, a_tp31, 2);

				i = 0;
				a_wr[0] = 0x04;
				while (i < 2) {
					if (tms37145_pwm_wr_ls_rd(1, a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
						i = 0;
					else
						i++;
				}
///led_off();
				lcd_print_blank(1);
				result = TRUE;
				state = s_exit;
				break;

			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}

	return result;
}
#endif
/*
void ts0d_unlock_tag_init(void)
{
	lcd_clear_screen();
	lcd_print_str(0, 0, "TS0DUnlock V1.0", 15);
	//led_on();
//	delay_us(500);
//	led_off();
}
*/
uint8_t ts0d_unlock_tag(void)
{
	static const uint8_t a_tp29[4] = {0x00, 0x00, 0x00, 0x00};
	
	uint8_t result, i_fail, a_wr[2], a_rd[11];
	enum {
		s_wr_tp29, s_ok, s_exit
	} state;

	result = FALSE;
	state = s_wr_tp29;
	while (state != s_exit) {
		switch (state) {
			case s_wr_tp29:
				if (!ts0dnew_test_write(29, a_tp29))
					state = s_exit;
				else
					state = s_ok;
				break;

			case s_ok:
			//	led_on();
				lcd_print_hex(30, 40,200,24,24, a_tp29, 1);

				i_fail = 0;
				a_wr[0] = 0x04;
				while (i_fail < 2) {
					if (tms37145_pwm_wr_ls_rd(1, a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
						i_fail = 0;
					else
						i_fail++;
				}
			//	led_off();
				lcd_print_blank(1);
				result = TRUE;
				state = s_exit;
				break;

			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}

	return result;
}

#define TPX2_CMD_GET_PARA			(0xFC)
#define TPX2_CMD_SET_TYPE			(0xE1)
#define TPX2_TYPE_TAG60				(3)
#define TPX2_TYPE_TAG64				(7)

tpx2_tag_t tpx2_tag1;

uint8_t tpx2_get_parameter(tpx2_tag_t *p_tag)
{
	uint8_t a_wr[2], a_rd[10], *p_ori;
	
	a_wr[0] = TPX2_CMD_GET_PARA;
	p_tag->read_len = 0;
	if (!tms37145_pwm_wr_ls_rd(1, &a_wr[0], TMS37145_T_PWM_READ_RECH, a_rd))
		;
	else if (tms37145_calc_crc16(&a_rd[1], 5) == (a_rd[6] | ((uint16_t)a_rd[7]<<8)))
		p_tag->read_len = 8;
	else if (tms37145_calc_crc16(&a_rd[1], 7) == (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		p_tag->read_len = 10;
	
	if (p_tag->read_len == 10)
		p_ori = &a_rd[3];
	else if (p_tag->read_len == 8)
		p_ori = &a_rd[1];
	else
		return FALSE;

	p_tag->para[0] = (p_ori[0]^p_ori[1]) | 0xEE;		// 0x77
	p_tag->para[1] = (p_ori[2]^p_ori[3]) | 0x9E;		// 0x79
	p_tag->para[2] = (p_ori[1]|p_ori[2]) ^ 0xE9;		// 0x97
	p_tag->para[3] = (p_ori[0]|p_ori[3]) ^ 0x99;		// 0x99

	a_wr[0] = 0x04;
	if (tms37145_pwm_wr_ls_rd(1, &a_wr[0], TMS37145_T_PWM_READ_RECH, a_rd)) {
		p_tag->psw = a_rd[1];
	}
	else
		return FALSE;

	return TRUE;
}

/* TPX2 is OTP, can copy one time */
uint8_t tpx2_copy_b9wk(const tms37145_tag_t *p_b9wk)
{
		uint8_t result, i;
	uint8_t a_wr[14], a_rd[10];
	if (!tpx2_get_parameter(&tpx2_tag1))
		return FALSE;

	result = FALSE;
	copy_bytes(&a_wr[1], tpx2_tag1.para, 4);
	// write page 1
	a_wr[0] = 0x05;
	i = 5;
	if (tpx2_tag1.psw != 0xFF)
		a_wr[i++] = tpx2_tag1.psw;
	a_wr[i++] = p_b9wk->page1[0];
	a_wr[i++] = 0;
	a_wr[i++] = 0;
	if (tms37145_pwm_wr_ls_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd)) {
		result = TRUE;
		tpx2_tag1.psw = a_rd[1];
	}
	// write page2
	if (result) {
		result = FALSE;
		a_wr[0] = 0x09;
		i = 5;
		if (tpx2_tag1.psw != 0xFF)
			a_wr[i++] = tpx2_tag1.psw;
		a_wr[i++] = p_b9wk->page2[0];
		a_wr[i++] = 0;
		a_wr[i++] = 0;
		if (tms37145_pwm_wr_ls_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
			result = TRUE;
	}
	// write page3
	if (result) {
		result = FALSE;
		a_wr[0] = 0x0D;
		i = 5;
		if (tpx2_tag1.psw != 0xFF)
			a_wr[i++] = tpx2_tag1.psw;
		a_wr[i++] = p_b9wk->page3[3];
		a_wr[i++] = p_b9wk->page3[2];
		a_wr[i++] = p_b9wk->page3[1];
		a_wr[i++] = p_b9wk->page3[0];
		a_wr[i++] = 0;
		a_wr[i++] = 0;
		if (tms37145_pwm_wr_ls_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
			result = TRUE;
	}
	// write page4
	if (result) {
		result = FALSE;
		a_wr[0] = 0x11;
		i = 5;
		if (tpx2_tag1.psw != 0xFF)
			a_wr[i++] = tpx2_tag1.psw;
		a_wr[i++] = p_b9wk->page4[4];
		a_wr[i++] = p_b9wk->page4[3];
		a_wr[i++] = p_b9wk->page4[2];
		a_wr[i++] = p_b9wk->page4[1];
		a_wr[i++] = p_b9wk->page4[0];
		a_wr[i++] = 0;
		a_wr[i++] = 0;
		if (tms37145_pwm_wr_ls_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
			result = TRUE;
	}

	/* change mode(60 or 64) */
	if (result) {
		result = FALSE;
		a_wr[0] = TPX2_CMD_SET_TYPE;
		i = 5;
		if (tpx2_tag1.psw != 0xFF)
			a_wr[i++] = tpx2_tag1.psw;
		a_wr[i++] = 0;
		a_wr[i++] = (p_b9wk->read_len == 10) ? TPX2_TYPE_TAG60: TPX2_TYPE_TAG64;
		a_wr[i++] = 0;
		a_wr[i++] = 0;
		a_wr[i++] = 0;
		a_wr[i++] = 0;
		result = (tms37145_pwm_wr_ls_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd));
	}

	/* check */
	if (result) {
		tms37145_find_tag(&tms37145_tag_copy);
		result = tms37145_compare_data(&tms37145_tag_copy, p_b9wk);
	}

	#if 1
	// lock page 1
	if (result && (p_b9wk->page1[1] == e_page_lock)) {
		result = FALSE;
		a_wr[0] = 0x06;
		i = 5;
		if (tpx2_tag1.psw != 0xFF)
			a_wr[i++] = tpx2_tag1.psw;
		a_wr[i++] = 0;
		a_wr[i++] = 0;
		if (tms37145_pwm_wr_ls_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
			result = TRUE;
	}
	// lock page 2
	if (result && (p_b9wk->page2[1] == e_page_lock)) {
		result = FALSE;
		a_wr[0] = 0x0A;
		i = 5;
		if (tpx2_tag1.psw != 0xFF)
			a_wr[i++] = tpx2_tag1.psw;
		a_wr[i++] = 0;
		a_wr[i++] = 0;
		if (tms37145_pwm_wr_ls_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
			result = TRUE;
	}
	// lock page 3
	if (result && (p_b9wk->page3[4] == e_page_lock)) {
		result = FALSE;
		a_wr[0] = 0x0E;
		i = 5;
		if (tpx2_tag1.psw != 0xFF)
			a_wr[i++] = tpx2_tag1.psw;
		a_wr[i++] = 0;
		a_wr[i++] = 0;
		if (tms37145_pwm_wr_ls_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
			result = TRUE;
	}
	// lock page 4
	if (result && (p_b9wk->page4[5] == e_page_lock)) {
		result = FALSE;
		a_wr[0] = 0x12;
		i = 5;
		if (tpx2_tag1.psw != 0xFF)
			a_wr[i++] = tpx2_tag1.psw;
		a_wr[i++] = 0;
		a_wr[i++] = 0;
		if (tms37145_pwm_wr_ls_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
			result = TRUE;
	}
	#endif

	return result;
}

uint8_t ts0d_read_sn(uint8_t * const p_sn)
{
	uint8_t a_wr[6], a_rd[11];
	uint8_t result;

	/* sn1 */
	result = FALSE;
	a_wr[0] = 0x98;
	a_wr[1] = 0x67;
	a_wr[2] = 0xF0;
	if (tms37145_pwm_wr_ls_crc_rd(3, &a_wr[0], TMS37145_T_PROG, a_rd)) {
		result = TRUE;
		copy_bytes(&p_sn[0], &a_rd[1], 8);
	}

	/* sn2 */
	if (result) {
		result = FALSE;
		a_wr[0] = 0x98;
		a_wr[1] = 0x67;
		a_wr[2] = 0x0F;
		if (tms37145_pwm_wr_ls_crc_rd(3, &a_wr[0], TMS37145_T_PROG, a_rd)) {
			result = TRUE;
			copy_bytes(&p_sn[8], &a_rd[1], 8);
		}
	}

	/* sync */
	if (result) {
		result = FALSE;
		a_wr[0] = 0x9A;
		a_wr[1] = 0x65;
		if (tms37145_pwm_wr_ls_crc_rd(2, &a_wr[0], TMS37145_T_PROG, a_rd)) {
			result = TRUE;
			copy_bytes(&p_sn[16], &a_rd[1], 8);
		}
	}

	/* dco */
	if (result) {
		result = FALSE;
		a_wr[0] = 0x9C;
		a_wr[1] = 0x63;
		if (tms37145_pwm_wr_ls_crc_rd(2, &a_wr[0], TMS37145_T_PROG, a_rd)) {
			result = TRUE;
			copy_bytes(&p_sn[24], &a_rd[1], 8);
		}
	}

	return result;
}

uint8_t ts0d_copy_b9wk(const tms37145_tag_t *p_b9wk)
{
	uint8_t a_wr[19], a_sn[32];

	delay_us(500*1000);
	if (!ts0d_read_sn(a_sn))
		return FALSE;

	a_wr[0] = 0x9D;
	a_wr[1] = 0x62;
	
	a_wr[2] = (p_b9wk->read_len == 10) ? 0x7B : 0x7D;
	a_wr[2] ^= (a_sn[0] ^ a_sn[30]);

	a_wr[3] = 0x00;
	if (p_b9wk->page1[1] == e_page_lock)
		a_wr[3] |= 0x01;
	if (p_b9wk->page2[1] == e_page_lock)
		a_wr[3] |= 0x02;
	if (p_b9wk->page3[4] == e_page_lock)
		a_wr[3] |= 0x04;
	if (p_b9wk->page4[5] == e_page_lock)
		a_wr[3] |= 0x08;
	a_wr[3] ^= (a_sn[8] ^ a_sn[25]);

	a_wr[4] = p_b9wk->page1[0];
	a_wr[4] ^= (a_sn[1] ^ a_sn[20]);

	a_wr[5] = p_b9wk->page2[0];
	a_wr[5] ^= (a_sn[2] ^ a_sn[17]);

	a_wr[6] = p_b9wk->page3[3];
	a_wr[6] ^= (a_sn[9] ^ a_sn[19]);

	a_wr[7] = p_b9wk->page3[2];
	a_wr[7] ^= (a_sn[10] ^ a_sn[29]);

	a_wr[8] = p_b9wk->page3[1];
	a_wr[8] ^= (a_sn[3] ^ a_sn[16]);

	a_wr[9] = p_b9wk->page3[0];
	a_wr[9] ^= (a_sn[11] ^ a_sn[18]);

	a_wr[10] = p_b9wk->page4[4];
	a_wr[10] ^= (a_sn[12] ^ a_sn[27]);

	a_wr[11] = p_b9wk->page4[3];
	a_wr[11] ^= (a_sn[4] ^ a_sn[22]);

	a_wr[12] = p_b9wk->page4[2];
	a_wr[12] ^= (a_sn[21] ^ a_sn[13]);

	a_wr[13] = p_b9wk->page4[1];
	a_wr[13] ^= (a_sn[5] ^ a_sn[24]);

	a_wr[14] = p_b9wk->page4[0];
	a_wr[14] ^= (a_sn[6] ^ a_sn[28]);

	a_wr[15] = a_sn[16];
	a_wr[16] = a_sn[30];

	if (tms37145_pwm_wr_ls_crc_rd(17, &a_wr[0], TMS37145_T_PROG, a_sn))
		if (a_sn[1] == p_b9wk->page1[0])
			return TRUE;

	return  FALSE;
}
 
uint8_t ts4d_unlock_all(void)
{
	uint8_t len;
	uint8_t a_wr[10], a_rd[11];
	ts4d_tag_t tag;

	/* get 6 page data */
	a_wr[0] = 0x04;
	if (!tms37145_pwm_wr_ls_rd(1, a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
		return FALSE;
	tag.page1 = a_rd[1];
	tag.page2 = a_rd[2];
	copy_bytes(tag.page3, &a_rd[3], 4);

	a_wr[0] = 0x1C;
	if (!tms37145_pwm_wr_ls_rd(1, a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
		return FALSE;
	copy_bytes(tag.page4, &a_rd[1], 5);
	tag.page5 = a_rd[6];
	tag.page6 = a_rd[7];

	/* reset all page to 0x00 */
	a_wr[0] = 0x1D;
	len = 1;
	if (tag.page1 != 0xFF)
		a_wr[len++] = tag.page1;
	a_wr[len++] = 0x00;
	tms37145_pwm_wr_ls_crc_rd(len, a_wr, TMS37145_T_PROG, a_rd);

	/* write page 6, 5 */
	a_wr[0] = 0x19;
	a_wr[1] = 0x00;
	a_wr[2] = tag.page6;
	tms37145_pwm_wr_ls_crc_rd(3, a_wr, TMS37145_T_PROG, a_rd);
	
	a_wr[0] = 0x15;
	a_wr[1] = 0x00;
	a_wr[2] = tag.page5;
	tms37145_pwm_wr_ls_crc_rd(3, a_wr, TMS37145_T_PROG, a_rd);
	
	/* write page 4, 3, 2, 1 */
	a_wr[0] = 0x11;
	a_wr[1] = 0x00;
	copy_bytes(&a_wr[2], tag.page4, 5);
	if (!tms37145_pwm_wr_ls_crc_rd(7, a_wr, TMS37145_T_PROG, a_rd))
		return FALSE;
	else if (a_rd[7] != a_wr[0])
		return FALSE;

	a_wr[0] = 0x0D;
	a_wr[1] = 0x00;
	copy_bytes(&a_wr[2], tag.page3, 4);
	if (!tms37145_pwm_wr_ls_crc_rd(6, a_wr, TMS37145_T_PROG, a_rd))
		return FALSE;
	else if (a_rd[7] != a_wr[0])
		return FALSE;

	a_wr[0] = 0x09;
	a_wr[1] = 0x00;
	a_wr[2] = tag.page2;
	if (!tms37145_pwm_wr_ls_crc_rd(3, a_wr, TMS37145_T_PROG, a_rd))
		return FALSE;
	else if (a_rd[7] != a_wr[0])
		return FALSE;

	a_wr[0] = 0x05;
	a_wr[1] = 0x00;
	a_wr[2] = tag.page1;
	if (!tms37145_pwm_wr_ls_crc_rd(3, a_wr, TMS37145_T_PROG, a_rd))
		return FALSE;
	else if (a_rd[7] != a_wr[0])
		return FALSE;

	return TRUE;
}

uint8_t ts804d_copy_b9wk(const tms37145_tag_t *p_b9wk)
{
	if (p_b9wk->read_len == 8)
		return FALSE;
	
	if (ts80_unlock_all())
		;
	else if (ts4d_unlock_all())
		;
	else
		return FALSE;

	return  b9wk_set_tag(p_b9wk, TRUE);
}

uint8_t ys80_copy_b9wk(const tms37145_tag_t *p_b9wk)
{
		uint8_t a_rd[11];
	uint8_t cmd = 0x0C;
	if (p_b9wk->read_len == 8)
		return FALSE;


	
	if (!tms37145_pwm_wr_ls_rd(1, &cmd, TMS37145_T_PWM_READ_RECH, a_rd))
		return FALSE;
	else if (a_rd[7] != 0x0C)
		return FALSE;

	return b9wk_set_tag(p_b9wk, FALSE);
}

/********************************************************
* furui Tag, cn2 tag, ys01 atag
* 4C/4D(not 64 mode) 
********************************************************/
uint8_t ys01_change_mode(uint8_t mode)
{
	uint8_t result;
	uint8_t a_wr[6], a_rd[10];

	a_wr[1]=0x39;
	a_wr[3]=0x4F;
	a_wr[4]=0x54;
	if (mode == 0x4D) {
		a_wr[0]=0x1D;
		a_wr[2]=0x42;  
		a_wr[5]=0x52;  
	}
	else if (mode == 0x4C) {
		a_wr[0]=0x19;
		a_wr[2]=0x52;  
		a_wr[5]=0x42;	
	}
	else
		return FALSE;

	tms37145_pwm_wr_ls_rd(6, a_wr, 20, a_rd);

	delay_us(20*1000);
	if (mode == 0x4D) {
		a_wr[0] = 0x04;
		result = tms37145_pwm_wr_ls_rd(1, a_wr, TMS37145_T_PWM_READ_RECH, a_rd);
	}
	else {
		r9wk_tag_t temp_tag;
		result = r9wk_find_tag(&temp_tag);
	}

	return result;
}

uint8_t ys01_unlock_page(uint8_t page)
{
	uint8_t a_wr[6], a_rd[10];

	if (page == 1)
		a_wr[0] = 0x16;
	else if (page == 2)
		a_wr[0] = 0x1A;
	else if (page == 3)
		a_wr[0] = 0x1E;
	else if (page == 4)
		a_wr[0] = 0x02;
	else
		return FALSE;

	a_wr[1] = 0x30 + page;
	a_wr[2] = 0x4E;
	a_wr[3] = 0x55;
	a_wr[4] = 0x53;
	a_wr[5] = 0x59;

	if (tms37145_pwm_wr_ls_rd(6, a_wr, 20, a_rd))
		if ((a_rd[7] & 0x03) == 0x00)
			return TRUE;

	return FALSE;
}

uint8_t ys01_copy_b9wk(const tms37145_tag_t *p_b9wk)
{
	uint8_t i;
	if (p_b9wk->read_len == 8)
		return FALSE;
	
	if (!ys01_change_mode(0x4D))
		return FALSE;
	for (i = 1; i < 5; i++)
		if (!ys01_unlock_page(i))
			return FALSE;

	return b9wk_set_tag(p_b9wk, TRUE);
}

uint8_t ys01_copy_r9wk(const r9wk_tag_t *p_r9wk)
{
	uint8_t i, result, psw;
	uint8_t a_wr[8], a_rd[10];

	if (!ys01_change_mode(0x4D))
		return FALSE;

	for (i = 2; i < 5; i++)
		if (!ys01_unlock_page(i))
			return FALSE;

	a_wr[0] = 0x04;
	if (tms37145_pwm_wr_ls_rd(1, a_wr, TMS37145_T_PWM_READ_RECH, a_rd))
		psw = a_rd[1];
	else
		return FALSE;

	/* write page2 */
	result = FALSE;
	i = 0;
	a_wr[i++] = 0x09;
	if (psw != 0xFF)
		a_wr[i++] = psw;
	a_wr[i++] = p_r9wk->a_id[0];
	if (tms37145_pwm_wr_ls_crc_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
		if (a_rd[7] == a_wr[0])
			result = TRUE;
	/* write page3 */
	if (result) {
		result = FALSE;
		i = 0;
		a_wr[i++] = 0x0D;
		if (psw != 0xFF)
			a_wr[i++] = psw;
		a_wr[i++] = p_r9wk->a_id[1];
		a_wr[i++] = p_r9wk->a_id[2];
		a_wr[i++] = p_r9wk->a_id[3];
		a_wr[i++] = p_r9wk->a_id[4];
		if (tms37145_pwm_wr_ls_crc_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
			if (a_rd[7] == a_wr[0])
				result = TRUE;
	}
	/* write page4 */
	if (result) {
		result = FALSE;
		i = 0;
		a_wr[i++] = 0x11;
		if (psw != 0xFF)
			a_wr[i++] = psw;
		a_wr[i++] = p_r9wk->a_id[5];
		a_wr[i++] = p_r9wk->a_id[6];
		a_wr[i++] = p_r9wk->a_id[7];
		a_wr[i++] = p_r9wk->a_id[8];
		a_wr[i++] = p_r9wk->a_id[9];
		if (tms37145_pwm_wr_ls_crc_rd(i, &a_wr[0], TMS37145_T_PROG, a_rd))
			if (a_rd[7] == a_wr[0])
				result = TRUE;
	}

	if (result)
		result = ys01_change_mode(0x4C);

	return result;
}

uint8_t b9wk60_comm_handle(const uint8_t *p_ask, uint8_t *p_answer)
{
	uint8_t len_send = 1;
	p_answer[0] = 0x00;
	
	if (p_ask[0] == 0x01) {
		if (tms37145_pwm_wr_ls_rd(p_ask[1], &p_ask[2], p_ask[2+p_ask[1]], &p_answer[1])) {
			len_send = 1 + 10;
			p_answer[0] = p_ask[0];
		}
	}
	else if (p_ask[0] == 0x02) {
		if (tms37145_pwm_wr_ls_crc_rd(p_ask[1], &p_ask[2], p_ask[2+p_ask[1]], &p_answer[1])) {
			len_send = 1 + 10;
			p_answer[0] = p_ask[0];
		}
	}
	else if (p_ask[0] == 0x05) {
		if (ts4d_unlock_all())
			p_answer[0] = p_ask[0];
	}

	if (len_send != 1) {
		/* remove b9wk64 tag case */
		if (tms37145_calc_crc16(&p_answer[2], 5) == (p_answer[7] | ((uint16_t)p_answer[8]<<8))) {
			len_send = 1;
			p_answer[0] = 0x00;
		}
		/* remove CRC invalid case */
		else if (tms37145_calc_crc16(&p_answer[2], 7) != (p_answer[9] | ((uint16_t)p_answer[10]<<8))) {
			len_send = 1;
			p_answer[0] = 0x00;
		}
	}

	return len_send;
}

uint8_t b9wk64_comm_handle(const uint8_t *p_ask, uint8_t *p_answer)
{
	uint8_t len_send = 1;
	p_answer[0] = 0x00;
	
	if (p_ask[0] == 0x01) {
		if (tms37145_pwm_wr_ls_rd(p_ask[1], &p_ask[2], p_ask[2+p_ask[1]], &p_answer[1])) {
			len_send = 1 + 8;
			p_answer[0] = p_ask[0];
		}
	}
	else if (p_ask[0] == 0x02) {
		if (tms37145_pwm_wr_ls_crc_rd(p_ask[1], &p_ask[2], p_ask[2+p_ask[1]], &p_answer[1])) {
			len_send = 1 + 8;
			p_answer[0] = p_ask[0];
		}
	}

	if (len_send != 1) {
		if (tms37145_calc_crc16(&p_answer[2], 5) != (p_answer[7] | ((uint16_t)p_answer[8]<<8))) {
			len_send = 1;
			p_answer[0] = 0x00;
		}
	}

	return len_send;
}

uint8_t tms37145_comm_handle(const uint8_t *p_ask, uint8_t *p_answer)
{
	uint8_t len_send = 1;
	p_answer[0] = 0x00;
	
	/* PWM without CRC */
	if (p_ask[0] == 0x01) {
		if (tms37145_pwm_wr_ls_rd(p_ask[1], &p_ask[2], p_ask[2+p_ask[1]], &p_answer[1])) {
			len_send = 1 + 10;
			p_answer[0] = p_ask[0];
		}
	}
	/* PWM add CRC */
	else if (p_ask[0] == 0x02) {
		if (tms37145_pwm_wr_ls_crc_rd(p_ask[1], &p_ask[2], p_ask[2+p_ask[1]], &p_answer[1])) {
			len_send = 1 + 10;
			p_answer[0] = p_ask[0];
		}
	}
	/* PPM without CRC */
	else if (p_ask[0] == 0x03) {
		if (tms37145_ppm_wr_ls_rd(p_ask[1], &p_ask[2], p_ask[2+p_ask[1]], &p_answer[1])) {
			len_send = 1 + 10;
			p_answer[0] = p_ask[0];
		}
	}
	/* PPM add CRC */
	else if (p_ask[0] == 0x04) {
		if (tms37145_ppm_wr_ls_crc_rd(p_ask[1], &p_ask[2], p_ask[2+p_ask[1]], &p_answer[1])) {
			len_send = 1 + 10;
			p_answer[0] = p_ask[0];
		}
	}
	/* PWM without CRC, times can change */
	else if (p_ask[0] == 0x11) {
		if (tms37145_pwmtime_wr_ls_rd(p_ask[1], &p_ask[2], p_ask[2+p_ask[1]], &p_answer[1])) {
			len_send = 1 + 10;
			p_answer[0] = p_ask[0];
		}
	}
	/* PWM add CRC, times can change */
	else if (p_ask[0] == 0x12) {
		if (tms37145_pwmtime_wr_ls_crc_rd(p_ask[1], &p_ask[2], p_ask[2+p_ask[1]], &p_answer[1])) {
			len_send = 1 + 10;
			p_answer[0] = p_ask[0];
		}
	}
	/* PPM without CRC, times can change */
	else if (p_ask[0] == 0x13) {
		if (tms37145_ppmtime_wr_ls_rd(p_ask[1], &p_ask[2], p_ask[2+p_ask[1]], &p_answer[1])) {
			len_send = 1 + 10;
			p_answer[0] = p_ask[0];
		}
	}
	/* PPM add CRC, times can change */
	else if (p_ask[0] == 0x14) {
		if (tms37145_ppmtime_wr_ls_crc_rd(p_ask[1], &p_ask[2], p_ask[2+p_ask[1]], &p_answer[1])) {
			len_send = 1 + 10;
			p_answer[0] = p_ask[0];
		}
	}
	/* update PWM times */
	else if (p_ask[0] == 0x1E) {
		tms3705_update_pwm_time((uint16_t)p_ask[1]*10, (uint16_t)p_ask[2]*10, 
								(uint16_t)p_ask[3]*10, (uint16_t)p_ask[4]*10);
		p_answer[0] = p_ask[0];
	}
	/* update PPM times */
	else if (p_ask[0] == 0x1F) {
		tms3705_update_ppm_time((uint16_t)p_ask[1]*10, (uint16_t)p_ask[2]*10, 
								(uint16_t)p_ask[3]*10);
		p_answer[0] = p_ask[0];
	}

	if (len_send != 1) {
		/* remove b9wk64 tag case */
		if (tms37145_calc_crc16(&p_answer[2], 5) == (p_answer[7] | ((uint16_t)p_answer[8]<<8))) {
			len_send = 1;
			p_answer[0] = 0x00;
		}
		/* remove CRC invalid case */
		else if (tms37145_calc_crc16(&p_answer[2], 7) != (p_answer[9] | ((uint16_t)p_answer[10]<<8))) {
			len_send = 1;
			p_answer[0] = 0x00;
		}
	}

	return len_send;
}


