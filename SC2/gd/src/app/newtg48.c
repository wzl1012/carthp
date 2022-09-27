#include "newtg48.h"


static void new48tg_7991_endcfg(void)
{
	pcf7991_inactive_driver();
	delay_us(5);
  pcf7991_set_page(0, 0x07/*PCF7991_PAGE0_EM4170*/);
	delay_us(1000);
	pcf7991_tagrd_gsetproc( );
		delay_us(1000);
	pcf7991_inactive_driver();
	delay_us(50*1000);
	//pcf7991_write_tag_n(0);
}
static void newtg48_power_control(uint8_t flag)
{
	if (flag) {
		pcf7991_active_driver();
		delay_us(20000);
		//delay_us(NEWTG48_CHARGE*1000);
	}
	else {
		pcf7991_inactive_driver();
		delay_us(20*1000);
	}
}
static void newtg48_wr_msbits(const uint8_t *p_wr, const uint8_t bits)
{
	uint8_t i, byte=0;
	
	for (i = 0; i < bits; i++) {
		pcf7991_driver_on();
		if ((i % 8) == 0)
			byte = *p_wr++;
		
		timer1_stop();
		TCNT1 = 65535-(2 * 2);	//adjust time, see assembler code
		timer1_start_8();

		while (TCNT_V < (20 * 2));

		/* MSBit first */
		if ((byte & 0x80) == 0)
			pcf7991_driver_off();		
		while (TCNT_V < (128 * 2));
		
		pcf7991_driver_on();		
		while (TCNT_V < (277 * 2));

		byte <<= 1;
	}

	timer1_stop();
}
static uint8_t newtg48_find_liw(void)
{
	uint8_t result;
	uint16_t tcnt_pre=0;
	enum {
		miss_high, miss_low,
		low_256us, high_384us, low_128us, high_512us,
		find_liw
	} state;
	
	timer1_stop();
	timer1_clear_ov();
	TCNT1 =0xFFFF;			// find time 32768 us
	timer1_start_8();

	state = miss_high;
	result = FALSE;
	while ((!result) && (timer1_ov_flag == 0)) {
		switch (state) {
			case miss_high:
				if (pcf7991_dout_level == 0)
					state = miss_low;
				break;
			case miss_low:
				if (pcf7991_dout_level) {
					tcnt_pre = TCNT_V;
					state = high_384us;
				}
				break;
			case high_384us:
				if (pcf7991_dout_level==0) {
					if (((TCNT_V-tcnt_pre)>=(334*2)) && ((TCNT_V-tcnt_pre)<=(434*2))){
						tcnt_pre = TCNT_V;
						state = low_128us;
					}
					else {
						state = miss_low;
					}
				}
				break;
				case low_128us:
				if (pcf7991_dout_level) {
					if (((TCNT_V-tcnt_pre)>=(96*2)) && ((TCNT_V-tcnt_pre)<=(160*2))){
						tcnt_pre = TCNT_V;
						state = high_512us;
					}
					else
						state = miss_low;
				}
				break;
				case high_512us:
				if (pcf7991_dout_level==0) {
					if (((TCNT_V-tcnt_pre)>=(462*2)) && ((TCNT_V-tcnt_pre)<=(562*2))){
						tcnt_pre = TCNT_V;
						state = low_256us;
				}
					else {
						state = miss_low;
					}
				}
				break;
				case low_256us:
				if (pcf7991_dout_level) {
					if (((TCNT_V-tcnt_pre)>=(206*2)) && ((TCNT_V-tcnt_pre)<=(306*2))){
						state = find_liw;
					}
					else
						state = miss_low;
				}
				break;
			case find_liw:
				result = TRUE;
				break;
			default :
				break;
		}
	}

	timer1_stop();
	timer1_clear_ov();

	return result;
}
/*
static uint8_t new48tg_7991_cfg(void)
{
	uint8_t i, result;
  newtg48_power_control(TRUE);
	pcf7991_set_page(0, 0x07);
	delay_us(5);
	pcf7991_write_tag_n(0);
	delay_us(1000);
#if SYSTEM_SUPPORT_OS	
 taskENTER_CRITICAL();  
#endif
	pcf7991_read_tag();
	result = FALSE;
	if (!newtg48_find_liw()) {
		newtg48_power_control(FALSE);
		  return result;
	}
	//pcf7991_write_tag_n(0);
}*/
static bool pcf7991_48tagdecode_miss_low(uint8_t cmd)
{
	uint8_t i=0,templevel=0,perid=16;
	timer1_stop();
	timer1_clear_ov();
	TCNT1 = 65535/*0x0000*/;				// max time is 32.768 ms
	timer1_start_8();
		while (pcf7991_dout_level && (timer1_ov_flag == 0));
		if (timer1_ov_flag) {
			timer1_stop();
			timer1_clear_ov();
			return FALSE;
		}
		//if(cmd==0x3C){
			//perid=14;
		//}
		templevel=pcf7991_dout_level;
	while(i<perid&&(timer1_ov_flag == 0)){
		if(pcf7991_dout_level!=templevel){
			templevel=pcf7991_dout_level;
			 i++;
		}
	}
   pcf7991_driver_off();	
	timer1_stop();
	if (timer1_ov_flag) {
		timer1_clear_ov();
		return FALSE;
	}
	return TRUE;
}
static bool pcf7991_48tagdecode_mc(const uint8_t bit_period, uint8_t *p_rd, const uint8_t bits, uint8_t dir)
{
		uint8_t i;		// index of decode bit
	uint8_t dout_start=0;
	uint16_t tcnt_pre=0;
	const uint16_t eighth = ((uint16_t)bit_period) << 1;	//eighth of bit period time
	enum {
		s_start,s_startbitrd,s_decide_pre, s_front, s_behind,
		s_bit_ok, s_error
	} state;
	/* check bit period correct */
	switch (bit_period) {
		case 16:
		case 32:
		case 40:
		case 64:
			break;
		default :
			return FALSE;
	}
	
	i = 0;
	state = s_start;
	timer1_clear_ov();
	//TCNT1 = (/*65536 -*/ DECODE_MAX_T);
	/*timer1_start_8();
	while(pcf7991_dout_level==0&&TCNT_V<(eighth<<2));
			if(pcf7991_dout_level==0)
				return FALSE;
			else{
							if (dir == e_dir_msbit)
								*p_rd &= (~(1 << (7-(i%8))));
							  *p_rd |= (1 << (7-(i%8)));
							else
								*p_rd &= (~(1 << (i%8)));
							 *p_rd |= (1 << (i%8));
			}
		while(pcf7991_dout_level&& (timer1_ov_flag == 0));
      if (timer1_ov_flag) {
			timer1_stop();
			timer1_clear_ov();
			return FALSE;
		}*/
	while ((i < (bits)) && (timer1_ov_flag==0)) {
		switch (state) {
			case s_start:
				timer1_stop();
				timer1_clear_ov();
				TCNT1 = (/*65536 -*/ DECODE_MAX_T);
				timer1_start_8();
			 // pcf7991_driver_off(); 
			  //debug_printf("\r\n t2cnt=0x%x",TCNT_V);			
				state = s_decide_pre;
				break;			
			case s_decide_pre:
				if ((TCNT_V/*-(65536 - DECODE_MAX_T)*/) >= (eighth << 1)) {
						//debug_printf("\r\n t2cnt=0x%x",TCNT_V);
					// pcf7991_driver_on(); 
					dout_start = pcf7991_dout_level;
					state = s_front;
				}
				break;
			case s_front:
				/* V2.23:(eighth * 8); */
				/* V2.24:(eighth * 6); */
				if ((TCNT_V/*-(65536 -DECODE_MAX_T)*/) >= (eighth * 8))
					state = s_error;
				else if (dout_start != pcf7991_dout_level) {
					tcnt_pre = TCNT_V;				// synch clock
					state = s_behind;
				}
				break;
			case s_behind:
				/* remove abnormal */
				if ((TCNT_V-tcnt_pre) >= eighth) {
					if (dout_start != pcf7991_dout_level) {
						  // pcf7991_driver_off();    
						if (pcf7991_dout_level) {
							if (dir == e_dir_msbit)
								//*p_rd &= (~(1 << (7-(i%8))));
							  *p_rd |= (1 << (7-(i%8)));
							else
								//*p_rd &= (~(1 << (i%8)));
							*p_rd |= (1 << (i%8));
						}
						else {
							if (dir == e_dir_msbit)
								//*p_rd |= (1 << (7-(i%8)));
						   *p_rd &= (~(1 << (7-(i%8))));
							else
								//*p_rd |= (1 << (i%8));
							*p_rd &= (~(1 << (i%8)));
						}
						if ((i & 7) == 7)
							p_rd++;
						state = s_bit_ok;
					}
					else
						state = s_error;
				}
				break;
			case s_bit_ok:
				if ((TCNT_V-tcnt_pre) >= (eighth<<2)) {
					//pcf7991_driver_on();
					i++;
					state = s_start;
				}
				break;
			case s_error:
				timer1_stop();
				timer1_clear_ov();
				return FALSE;
			default :
				break;
		}
	}

	timer1_stop();
	timer1_clear_ov();

	/* correct decode bits data, return TRUE */
	if (i == bits)
		return TRUE;

	return FALSE;
}
static uint8_t newtg48_id_um_mode(uint8_t cmd, uint8_t *p_rd)
{
		uint8_t bytes, result,bits,a_read[11]={0};
  newtg48_power_control(TRUE);
	pcf7991_set_page(0, 0x07/*PCF7991_PAGE0_EM4170*/);
	delay_us(5);
	pcf7991_write_tag_n(0);
	delay_us(1000);
#if SYSTEM_SUPPORT_OS	
 taskENTER_CRITICAL();  
#endif
	pcf7991_read_tag();
	result = FALSE;
		/* find 2 times */
	if (!newtg48_find_liw()) {
		newtg48_power_control(FALSE);
		  return result;
	}
#if SYSTEM_SUPPORT_OS	
 taskEXIT_CRITICAL();  
#endif
	/* check command */
	switch (cmd) {
		case EM4170_CMD_ID_MODE:
		case EM4170_CMD_UM_MODE_1:
			bytes = 4;
		  bits=8;
			break;
		case EM4170_CMD_UM_MODE_2:
			bytes = 8;
		  bits=8;
			break;
		default:
			return FALSE;
	}
	//pcf7991_confg();
	//new48tg_7991_cfg();
	cmd >>= 2;
		result = FALSE;
	delay_us(400);
			pcf7991_write_tag_n(0);
		delay_us(400);
#if SYSTEM_SUPPORT_OS	
 taskENTER_CRITICAL();  
#endif
		newtg48_wr_msbits(&cmd, 6);
		pcf7991_tagrd_fsetproc();
		pcf7991_read_tag();
		if (!pcf7991_48tagdecode_miss_low(cmd))
			;
		else if (!pcf7991_48tagdecode_mc(32, a_read, (bytes*8 +bits), e_dir_msbit)/*pcf7991_decode_bp(32, a_read, (bytes*8 + 8), e_dir_msbit)*/)
			;
		/* check 16 bits header */
		else if ((a_read[0]&0xE0)==0xE0 /*0xFA*//*0x00*/) {
			copy_bytes(p_rd, &a_read[1], bytes);
			//copy_msbits(p_rd, 0, &a_read[1], 4, bytes*8);
			result = TRUE;
		}
#if SYSTEM_SUPPORT_OS	
 taskEXIT_CRITICAL();
#endif
			if (result)
		delay_us(1*1000);
	new48tg_7991_endcfg();		
	return result;
}

uint8_t new48_find_tag(em4170_tag_t *p_tag)
{
		
	uint8_t i,result;
	memset(p_tag,0,sizeof(em4170_tag_t));
	p_tag->mode = e_no_em4170;
	result = FALSE;
	//pcf7991_confg();
	//pcf7991_inactive_driver();
	//pcf7991_set_page(0, 0x07/*PCF7991_PAGE0_EM4170*/);
	//delay_us(1*1000);
	//	pcf7991_tagrd_gsetproc( );
	//delay_us(1*1000);
	//	pcf7991_inactive_driver();
	//delay_us(50*1000);
	for (i = 0; i < 2; i++) {
		if (!newtg48_id_um_mode(EM4170_CMD_ID_MODE, p_tag->a_id))
			continue;
		else if (!newtg48_id_um_mode(EM4170_CMD_UM_MODE_1, p_tag->a_um1))
			continue;
		else if (!newtg48_id_um_mode(EM4170_CMD_UM_MODE_2, p_tag->a_um2))
			continue;
		else
			break;
	}
			if (i < 2) {
		p_tag->mode = e_yes_em4170;
		result = TRUE;
	}

	newtg48_power_control(FALSE);
	return result;
}
