#include "headfile.h"
pcf7936_tag_t pcf7936_tag1;
pcf7936_tag_t pcf7936_tag_temp;

uint8_t pcf7936_t_charge;		// optimize read and write 46tag, charge time extend pcf7936_t_charge ms
uint8_t g_pcf7991_p0_pcf7936;

/********************************************************
* confirm variable pcf7936_t_charge befor use this function
********************************************************/
void pcf7936_off_to_wait(void)
{
	pcf7991_active_driver();
	delay_us(PCF7936_T_START);
	delay_us(PCF7936_T_INIT);
	/* In case one of the Read Only modes is enabled, the device enters READ ONLY state, if the first two bits of the
	START_AUTH command are not being recognized within the time-out period tWAIT,SA. */
	delay_us(PCF7936_T_WAIT_SA * 3 / 4);
	delay_us(pcf7936_t_charge*1000);
}

void pcf7936_to_off(void)
{
	pcf7991_inactive_driver();
	delay_us(PCF7936_T_RESET_MIN*1000);
}

/* Data Transmission timing */ 
#define PCF7936_T_WRP				(7* PCF7936_T_O)
#define PCF7936_T_LOG0				(20 * PCF7936_T_O)
#define PCF7936_T_LOG1				(30 * PCF7936_T_O)
#define PCF7936_T_STOP				(40 * PCF7936_T_O)

/* varialbel for taisu special test */
struct pcf7936_wr_time
{
	uint8_t off;
	uint8_t on0;
	uint8_t on1;
};
static struct pcf7936_wr_time g_wr_time = {PCF7936_T_WRP, 
										PCF7936_T_LOG0 - PCF7936_T_WRP,
										PCF7936_T_LOG1 - PCF7936_T_WRP};

static void update_wr_time(uint8_t off, uint8_t on0, uint8_t on1)
{
	g_wr_time.off = off << 3;
	g_wr_time.on0 = (on0 - off) << 3;
	g_wr_time.on1 = (on1 - off) << 3;
}

/********************************************************
* Data Transmission in Write Direction
********************************************************/
void pcf7936_wr_msbits(const uint8_t *p_wr, uint8_t bits, uint8_t stop_flag)
{
	uint8_t i = 0,byte=0;;
	for ( i = 0; i < bits; i++) {
		//uint8_t byte=0;
		if ((i % 8) == 0)
			byte = *p_wr++;

		pcf7991_driver_off();
	  //timer1_delay_us(g_wr_time.off);
		delay_us(g_wr_time.off);
		pcf7991_driver_on();
		//timer1_delay_us((byte & 0x80) ? g_wr_time.on1 : g_wr_time.on0);
		delay_us((byte & 0x80) ? g_wr_time.on1 : g_wr_time.on0);
		byte <<= 1;
	}

	/* check stop Condition */
	if (stop_flag) {
		pcf7991_driver_off();
		delay_us(g_wr_time.off);
		
		pcf7991_driver_on();
		//timer1_delay_us(PCF7936_T_STOP - PCF7936_T_WRP);
		delay_us(PCF7936_T_STOP - PCF7936_T_WRP);
		delay_us(100);
	}
}

/* keep in order for future */
/* 
uint8_t pcf7936_miss_eq(void)
{
	uint8_t eq_bits, result;
	uint16_t tcnt_pre;
	enum {
		s_reset_time, s_miss_high, s_miss_low, 
		s_bit_ok, s_error
	} state;

	result = FALSE;
	timer1_clear_ov();
	eq_bits = 0;
	state = s_reset_time;
	while ((result == FALSE) && (timer1_ov_flag == 0)) {
		switch (state) {
			case s_reset_time:
				timer1_stop();
				timer1_clear_ov();
				TCNT1 = 65535 - (256 * 2 * 2);
				timer1_start_8();
				state = s_miss_high;
				break;
			case s_miss_high:
				if (pcf7991_dout_level == 0) {
					delay_us(10);
					if (pcf7991_dout_level == 0)		// remove shake
						state = s_miss_low;
				}
				break;
			case s_miss_low:
				if (eq_bits == 4) {
					tcnt_pre = TCNT1;
					while ((TCNT1 - tcnt_pre) < 256);
					result = TRUE;
				}
				else if (pcf7991_dout_level) {
					delay_us(10);
					if (pcf7991_dout_level)		// remove shake
						state = s_miss_high;
				}
				break;
			default :
				break;
		}
	}

	timer1_stop();
	timer1_clear_ov();

	return result;
}
*/

/********************************************************
* send '11000' and receive ID
*
* return: if receive ID correct, return ture
********************************************************/
uint8_t pcf7936_auth_half(pcf7936_tag_t *p_tag)
{
		uint8_t a_read[5]={0},cmd;
	//if(p_tag->ictype==other_typeic)
	//	cmd = 0xE0;
//	else
	  cmd =0xC0/* 0xC0*/;
	uint8_t result = FALSE;
	switch (p_tag->tmcf_dcs) {
		case e_modu_mc:
		case e_modu_bp:
			break;
		default:
			return FALSE;
	}
	//pcf7991_write_tag_n(0);
	pcf7991_write_tag_n(0x07);
	delay_us(400);
	pcf7936_wr_msbits(&cmd,5, TRUE);
	//pcf7991_set_page(0x00, g_pcf7991_p0_pcf7936);	
	pcf7991_tagrd_fsetproc();
	pcf7991_read_tag();
#if SYSTEM_SUPPORT_OS	
 taskENTER_CRITICAL();  
#endif
	//delay_us(PCF7936_T_WAIT_TR);
	if (pcf7991_decode_miss_low()) {
		if (p_tag->tmcf_dcs == e_modu_mc)
			result = pcf7991_decode_mc(32, a_read, 36/*37*/, e_dir_msbit);
		else
			result = pcf7991_decode_bp(32, a_read, 36/*37*/, e_dir_msbit);
	}
#if SYSTEM_SUPPORT_OS	
 taskEXIT_CRITICAL();
#endif	
	/* check read correct */
	if (!result)
		delay_us(20);
   //delay_us(((256 * 37 / 1000) + PCF7936_T_IDLE_MAX)*1000);
	/* check header '11111' */
	else if ((a_read[0]&0xF0) == 0xF0/*0xF8*/) {
		p_tag->now_state = e_auth_half;
		copy_msbits(p_tag->a_page[0], 0, a_read, 4, 32);
		//copy_bytes(p_tag->a_page[0], a_read, 4);
	}
	else
		result = FALSE;
  pcf7991_exit_tag_mode( );
	return result;
}

/********************************************************
* send '11000' and receive ID
* send 32 bits password and receive page3
*
* return: if receive ID and page3 correct, return ture
********************************************************/
uint8_t pcf7936_auth_psw(pcf7936_tag_t *p_tag)
{
	uint8_t a_read[5];
	uint8_t result = FALSE;
  // pcf7936_auth_half(p_tag);
	//delay_us(400);
	//pcf7991_exit_tag_mode( );
	pcf7991_confg();
	if (pcf7936_auth_half(p_tag)){
		delay_us(PCF7936_T_WAIT_BS);
		//pcf7991_write_tag_n(0);
		pcf7991_write_tag_n(0x07);
		delay_us(400);
		pcf7936_wr_msbits(p_tag->a_page[1], 32, TRUE);
		//pcf7991_set_page(0x00, g_pcf7991_p0_pcf7936);
	  pcf7991_tagrd_fsetproc();
#if SYSTEM_SUPPORT_OS	
 taskENTER_CRITICAL();  
#endif
		pcf7991_read_tag();
		//delay_us(PCF7936_T_WAIT_TR);
		if (pcf7991_decode_miss_low()) {
			if (p_tag->tmcf_dcs == e_modu_mc)
				result = pcf7991_decode_mc(32, a_read, 36/*37*/, e_dir_msbit);
			else
				result = pcf7991_decode_bp(32, a_read, 36/*37*/, e_dir_msbit);
		}
#if SYSTEM_SUPPORT_OS	
 taskEXIT_CRITICAL();
#endif		
		if (result && ((a_read[0]&0xF0) == 0xF0/*0xF8*/)) {
			p_tag->now_state = e_authorized;
			copy_msbits(p_tag->a_page[3], 0, a_read,4/*5*/, 32);
      //copy_bytes(p_tag->a_page[3], a_read, 4);
		}
		else
			result = FALSE;
	}
		pcf7991_exit_tag_mode( );
	return result;
}

/********************************************************
* send '11000' and receive ID
* send 64 bits random and skey and receive page3
*
* return: if receive ID and page3 correct, return ture
********************************************************/
uint8_t pcf7936_auth_cipher(pcf7936_tag_t *p_tag)
{
	uint8_t result = FALSE;
	uint8_t a_write[4], a_read[5];
	pcf7991_confg();
	if (pcf7936_auth_half(p_tag)) {
		/*a_write[0] = 0xFF;
		a_write[1] = 0xFF;
		a_write[2] = 0xFF;
		a_write[3] = 0xFF;*/
		hitag2_1_set_id(p_tag->a_page[0], 32);
		//if(p_tag->hitagmode==hitag2){
		hitag2_2_set_sk(p_tag->secret_key ,48);
		hitag2_3_set_random(p_tag->random);
		//}else if(p_tag->hitagmode==hitag3){
			//hitag2_2_set_sk(p_tag->secret_key ,96);
			//hitag3_3_set_random(p_tag->random);
	//	}
		
		//hitag2_oneway2(a_write, 32);
				//if(p_tag->ictype==other_typeic){
        copy_bytes(a_write,p_tag->random,4);
					//a_write[0]=0xF9;
					//a_write[1]=0x41;
				//	hitag2_oneway2(a_write,16);
				//}else{
			   hitag2_oneway2(a_write,32);
				//}
			//for(uint8_t i=0;i<4;i++)
			// debug_printf("\r\nencya_write[%d]=0x%02x\r\n",i,a_write[i]);
		   delay_us(PCF7936_T_WAIT_BS);				
		//pcf7991_write_tag_n(0x07);
		//pcf7991_write_tag_n(0);
	//	if(p_tag->ictype==other_typeic){
		//  pcf7991_write_tag();
	//	}else{
			pcf7991_write_tag_n(0x07);
	//	}
		//if(p_tag->hitagmode==hitag2){
		pcf7936_wr_msbits(p_tag->random, 32, FALSE);	//send 32 bits random
		pcf7936_wr_msbits(a_write, 32, TRUE);			//send 32 bits skey
		//}else if(p_tag->hitagmode==hitag3){
		  //pcf7936_wr_msbits(p_tag->random, 64, TRUE);	//send 32 bits random	
		//}
		
		//pcf7991_set_page(0x00, g_pcf7991_p0_pcf7936);
		pcf7991_tagrd_fsetproc();
		/*if(p_tag->ictype==other_typeic){
		//hitag2_1_set_id(p_tag->a_page[0], 32);
		//hitag2_2_set_sk(p_tag->secret_key ,48);
		//hitag2_3_set_random(p_tag->random);
			//a_write[0]=p_tag->a_page[0][0];
			//a_write[1]=p_tag->a_page[0][1];
			//a_write[2]=p_tag->a_page[0][2];
			//a_write[3]=p_tag->a_page[0][3];
			//hitag2_oneway2(a_write, 32);
			delay_us(25000);
			pcf7991_write_tag_n(0x07);
			delay_us(400);
			pcf7936_wr_msbits(a_write, 16, TRUE);
			pcf7991_tagrd_fsetproc();
		}*/
#if SYSTEM_SUPPORT_OS	
 taskENTER_CRITICAL();  
#endif
		pcf7991_read_tag();
		//delay_us(PCF7936_T_WAIT_TR);
		if (pcf7991_decode_miss_low()) {
			if (p_tag->tmcf_dcs == e_modu_mc)
				result = pcf7991_decode_mc(32, a_read, 36/*37*/, e_dir_msbit);
			else
				result = pcf7991_decode_bp(32, a_read, 36/*37*/, e_dir_msbit);
		}
#if SYSTEM_SUPPORT_OS	
 taskEXIT_CRITICAL();
#endif		
		if (result && ((a_read[0]&0xF0) == 0xF0/*0xF8*/)) {
			p_tag->now_state = e_authorized;
			copy_msbits(p_tag->a_page[3], 0, a_read, 4, 32);
			//copy_bytes(p_tag->a_page[3], a_read, 4);
			hitag2_oneway2(p_tag->a_page[3], 32);		//receive 32 bit is encryption, so need decryption
		}
		else
			result = FALSE;
    pcf7991_exit_tag_mode( );
}
	return result;
}

/********************************************************
* function: decide pcf7936 modulation and updata tag data
*
* return: if read pcf7936 correct, return ture
********************************************************/
uint8_t pcf7936_decide_modu(pcf7936_tag_t *p_tag)
{
	uint8_t result;
	result=FALSE;
	p_tag->tmcf_dcs = e_modu_mc;
	//pcf7936_off_to_wait();
	pcf7991_confg();
	//result = pcf7936_auth_psw(p_tag);
	 result =pcf7936_auth_half(p_tag);
	pcf7936_to_off();
	if (!result) {
		p_tag->tmcf_dcs = e_modu_bp;
		//pcf7936_off_to_wait();
		pcf7991_confg();
		result = pcf7936_auth_half(p_tag);
		pcf7936_to_off();
	}
	//pcf7991_exit_tag_mode( );
	//delay_us(PCF7936_T_RESET_MIN*1000);
	//pcf7936_to_off();
	return result;
}

/********************************************************
* function: read pcf7936 one page data
*
* page: which page need read
* p_tag: pointer for store tag data
*
* return: if read pcf7936 one page success, return ture
********************************************************/
uint8_t pcf7936_read_page(uint8_t page, pcf7936_tag_t *p_tag)
{
		uint8_t cmd[2], a_read[5];
		uint8_t result = FALSE;
	switch (p_tag->tmcf_dcs) {
		case e_modu_mc:
		case e_modu_bp:
			break;
		default:
			return FALSE;
	}
	if (page > 7)
		return FALSE;
	
	cmd[0] = 0xC0 | (page<<3);	//see pcf7936 command
	if ((page&0x04) == 0)
		cmd[0] |= 0x01;
	cmd[1] = ((~page) << 6);

	
	delay_us(PCF7936_T_WAIT_BS);
	if (p_tag->tmcf_enc == e_pcf7936_cipher)
		hitag2_oneway2(cmd, 10);
	pcf7991_write_tag_n(0x07);
	//pcf7991_write_tag_n(0);
	delay_us(400);
	pcf7936_wr_msbits(&cmd[0], 10, TRUE);
	//pcf7991_set_page(0x00, g_pcf7991_p0_pcf7936);
	pcf7991_tagrd_fsetproc();
#if SYSTEM_SUPPORT_OS	
 taskENTER_CRITICAL();  
#endif
	pcf7991_read_tag(); 							// switching the device into transparent mode (READ_TAG mode)
	//delay_us(PCF7936_T_WAIT_TR);
	if (pcf7991_decode_miss_low()) {
		if (p_tag->tmcf_dcs == e_modu_mc)
			result = pcf7991_decode_mc(32, a_read, 36/*37*/, e_dir_msbit);
		else
			result = pcf7991_decode_bp(32, a_read, 36/*37*/, e_dir_msbit);
	}
#if SYSTEM_SUPPORT_OS	
 taskEXIT_CRITICAL();
#endif
	if (result && ((a_read[0]&0xF0) ==0xF0/* 0xF8*/)) {
		copy_msbits(p_tag->a_page[page], 0, a_read,4/* 5*/, 32);
		//copy_bytes(p_tag->a_page[page], a_read, 4);
		if (p_tag->tmcf_enc == e_pcf7936_cipher)
			hitag2_oneway2(p_tag->a_page[page], 32);
	}
	else
		result = FALSE;

	return result;
}

/********************************************************
* function: write pcf7936 one page data
*
* page: which page need write
* p_tag: pointer for tag
* p_wr: pointer for write data
*
* return: if write pcf7936 one page success, return ture
********************************************************/
uint8_t pcf7936_write_page(const uint8_t page, pcf7936_tag_t *p_tag, const uint8_t *p_wr)
{
		uint8_t cmd[2], a_read[5], a_write[5];
		uint8_t result =FALSE;
	switch (p_tag->tmcf_dcs) {
		case e_modu_mc:
		case e_modu_bp:
			break;
		default:
			return FALSE;
	}
	if (page > 7)
		return FALSE;
	
	cmd[0] = 0x82 | (page<<3);		//see pcf7936 command
	if ((page&0x04) == 0)
		cmd[0] |= 0x01;
	cmd[1] = ((~page) << 6);
	

	/* send write command, 10 bits */
	if (p_tag->tmcf_enc == e_pcf7936_cipher)
		hitag2_oneway2(cmd, 10);
	//pcf7991_write_tag_n(0);
	pcf7991_write_tag_n(0x07);
	pcf7936_wr_msbits(&cmd[0], 10, TRUE);
	pcf7991_set_page(0x00, g_pcf7991_p0_pcf7936);
	pcf7991_read_tag();
	delay_us(PCF7936_T_WAIT_TR);
	if (pcf7991_decode_miss_low()) {
		if (p_tag->tmcf_dcs == e_modu_mc)
			result = pcf7991_decode_mc(32, a_read, 15, e_dir_msbit);
		else
			result = pcf7991_decode_bp(32, a_read, 15, e_dir_msbit);
	}
	if (!result)
		return FALSE;

	/* send write data, 32 bits */
	cmd[0] = 0x00;
	cmd[1] = 0x00;
	copy_msbits(cmd, 0, a_read, 5, 10);
	copy_bytes(a_write, p_wr, 4);
	if (p_tag->tmcf_enc == e_pcf7936_cipher) {
		hitag2_oneway2(cmd, 10);
		hitag2_oneway2(a_write, 32);
	}
	delay_us(PCF7936_T_WAIT_BS);
	//pcf7991_write_tag_n(0);
	pcf7991_write_tag_n(0x07);
	pcf7936_wr_msbits(a_write, 32, TRUE);
	delay_us(PCF7936_T_PROG*1000);

	return TRUE;
}

/********************************************************
* function: read pcf7936 any page data
* start_page, start_page - 1, ... , end_page - 1, end_page
*
* p_tag: pointer for tag
* start_p: start page
* end_p: end page
*
* return: if read pcf7936 any page success, return ture
********************************************************/
uint8_t pcf7936_read_tag(pcf7936_tag_t *p_tag, uint8_t start_p, uint8_t end_p)
{
	uint8_t result;
	uint8_t trys = 0;
	uint8_t (* p_fun)(pcf7936_tag_t *);

	if (p_tag->tmcf_enc == e_pcf7936_cipher)
		p_fun = pcf7936_auth_cipher;
	else
		p_fun = pcf7936_auth_psw;
	
	result = FALSE;
	for (trys = 0; (trys<3) && (!result); trys++) {
		//pcf7936_off_to_wait();
		if (p_fun(p_tag)) {
			uint8_t i;
			for (i = start_p; i <= end_p; i++) {
				if (!pcf7936_read_page(i, p_tag))
					break;
			}
			if (i > end_p)
				result = TRUE;
			else
				result = FALSE;
		}
		pcf7936_to_off();
	}

	return result;
}

/********************************************************
* function: taisu 46 tag copy pcf7936 all page data
*
* p_tag_wr: pointer for write tag(taisu 46 tag)
* p_tag_rd: pointer for read tag(pcf7936 tag)
*
* return: if copy success, return ture
********************************************************/
uint8_t ts46_copy_pcf7936(pcf7936_tag_t *p_tag_wr, pcf7936_tag_t *p_tag_rd)
{
	uint8_t result, i;

	pcf7936_to_off();
	/* deicide code and crypt mode*/
	if (pcf7936_decide_modu(p_tag_wr))
		;
	else if (pcf7936_decide_modu(p_tag_wr))
		;
	else
		return FALSE;

	p_tag_wr->tmcf_enc = e_pcf7936_cipher;
	p_tag_wr->secret_key[0] = 0xFF;
	p_tag_wr->secret_key[1] = 0xFF;
	p_tag_wr->secret_key[2] = 0xFF;
	p_tag_wr->secret_key[3] = 0xFF;
	p_tag_wr->secret_key[4] = 0xFF;
	p_tag_wr->secret_key[5] = 0xFF;

	/* write 4-7 page */
	result = FALSE;
	pcf7936_off_to_wait();
	if (pcf7936_auth_cipher(p_tag_wr)) {
		for (i = 4; i < 8; i++) {
			delay_us(PCF7936_T_WAIT_BS);
			if (!pcf7936_write_page(i, p_tag_wr, p_tag_rd->a_page[i]))
				break;
		}
		result = (i == 8) ? TRUE : FALSE;
	}
	pcf7936_to_off();
	if (!result)
		return FALSE;

	/* write page0 */
	result = FALSE;
	pcf7936_off_to_wait();
	if (pcf7936_auth_cipher(p_tag_wr)) {
		delay_us(PCF7936_T_WAIT_BS);
		if (pcf7936_write_page(0, p_tag_wr, p_tag_rd->a_page[0])) {
			result = TRUE;
			copy_bytes(p_tag_wr->a_page[0], p_tag_rd->a_page[0], 4);
		}
	}
	pcf7936_to_off();
	if (!result)
		return FALSE;

	/* write page1 */
	result = FALSE;
	pcf7936_off_to_wait();
	if (pcf7936_auth_cipher(p_tag_wr)) {
		delay_us(PCF7936_T_WAIT_BS);
		if (pcf7936_write_page(1, p_tag_wr, p_tag_rd->a_page[1])) {
			result = TRUE;
			copy_bytes(&(p_tag_wr->secret_key[2]), p_tag_rd->a_page[1], 4);
		}
	}
	pcf7936_to_off();
	if (!result)
		return FALSE;

	/* write page2 */
	p_tag_rd->a_page[2][0] = 0x96;			// because taisu tag
	p_tag_rd->a_page[2][1] = 0xF8;
	result = FALSE;
	pcf7936_off_to_wait();
	if (pcf7936_auth_cipher(p_tag_wr)) {
		delay_us(PCF7936_T_WAIT_BS);
		if (pcf7936_write_page(2, p_tag_wr, p_tag_rd->a_page[2])) {
			result = TRUE;
			p_tag_wr->secret_key[0] = p_tag_rd->a_page[2][2];
			p_tag_wr->secret_key[1] = p_tag_rd->a_page[2][3];
		}
	}
	pcf7936_to_off();
	if (!result)
		return FALSE;

	/* write page3 */
	result = FALSE;
	pcf7936_off_to_wait();
	if (pcf7936_auth_cipher(p_tag_wr)) {
		delay_us(PCF7936_T_WAIT_BS);
		if (pcf7936_write_page(3, p_tag_wr, p_tag_rd->a_page[3])) {
			result = TRUE;
			if (p_tag_rd->a_page[3][0] & 0x01)
				p_tag_wr->tmcf_dcs = e_modu_bp;
			else
				p_tag_wr->tmcf_dcs = e_modu_mc;
			if (p_tag_rd->a_page[3][0] & 0x08)
				p_tag_wr->tmcf_enc = e_pcf7936_cipher;
			else
				p_tag_wr->tmcf_enc = e_pcf7936_psw;
		}
	}
	pcf7936_to_off();
	if (!result)
		return FALSE;

	/* for future use */
	/* checkout
	result = FALSE;
	if (pcf7936_read_tag(p_tag_wr, 3, 7)) {
		for (i = 3; i < 8; i++)
			if (!equal_bytes(p_tag_wr->a_page[i], p_tag_rd->a_page[i], 4))
				break;
		if (i < 8)
			;
		else if (equal_bytes(p_tag_wr->a_page[0], p_tag_rd->a_page[0], 4))
			result = TRUE;
	} */

	return result;
}
static uint8_t pcf7936_decide_crytmod(pcf7936_tag_t *p_tag)
{
		//static  uint8_t a_page1[4] = {0x4D, 0x49, 0x4B, 0x52};
	//static  uint8_t a_page2[4] = {0x96, 0xF8, 0x4F, 0x4E};
	//static const uint8_t a_page3[4] = {0x06, 0xAA, 0x48, 0x54};
	uint8_t i=2;
	if((p_tag->tmcf_enc!=e_pcf7936_psw)&&(p_tag->tmcf_enc!=e_pcf7936_cipher))
	  p_tag->tmcf_enc=e_pcf7936_psw;
  //if(p_tag->cartype>lastcartype)
    p_tag->cartype=defaultcartype;		
while(--i){
switch(p_tag->tmcf_enc){
	//if(p_tag->tmcf_enc==e_pcf7936_psw){
	case(e_pcf7936_psw):{
		while(p_tag->cartype<=lastcartype){
			  switch (p_tag->cartype){
						case(defaultcartype):{
					    p_tag->a_page[1][0] = 0x4D;
		          p_tag->a_page[1][1] = 0x49;
		          p_tag->a_page[1][2] = 0x4B;
		          p_tag->a_page[1][3] = 0x52;
			      }
						break;
				   case(hongda):{
					    p_tag->a_page[1][0] = 0x23;
		          p_tag->a_page[1][1] = 0x07;
		          p_tag->a_page[1][2] = 0x73;
		          p_tag->a_page[1][3] = 0x19;
			      }
			      break;
					 case(exc46):{
             	p_tag->a_page[1][0] = 0x23;
		          p_tag->a_page[1][1] = 0x07;
		          p_tag->a_page[1][2] = 0x73;
		          p_tag->a_page[1][3] = 0x19;
					 }						 
				 case(lastcartype):
					     return FALSE;
				 default:
					  p_tag->cartype=defaultcartype;
            p_tag->tmcf_enc=e_pcf7936_cipher;
            break;				 
			 }
     if(p_tag->cartype!=lastcartype){
        p_tag->cartype+=1;			 
	   	if(!pcf7936_auth_psw(p_tag)){
			      pcf7936_to_off();
			     //p_tag->tmcf_enc=e_pcf7936_cipher;			
		    }else{
					   pcf7936_to_off();
					   p_tag->tmcf_enc=e_pcf7936_psw;
					  debug_printf("\r\npsw_cartype=%d\r\n",p_tag->cartype);						
						return TRUE;
				 }
				}else{
					break;
				}
				//p_tag->cartype+=1;	
	   }
	 }
break;
case(e_pcf7936_cipher):{
			while(p_tag->cartype<=lastcartype){
			  switch (p_tag->cartype){
						case(defaultcartype):{
					    p_tag->secret_key[0] = 0x52;
		          p_tag->secret_key[1] = 0x4b;
		          p_tag->secret_key[2] = 0x49;
		          p_tag->secret_key[3] = 0x4d;
		          p_tag->secret_key[4] = 0x4E;
		          p_tag->secret_key[5] = 0x4F;
			      }
						break;
				   case(hongda):{
					    p_tag->secret_key[0] = 0x19;
		          p_tag->secret_key[1] = 0x73;
		          p_tag->secret_key[2] = 0x07;
		          p_tag->secret_key[3] = 0x23;
		          p_tag->secret_key[4] = 0x0A;
		          p_tag->secret_key[5] = 0x0E;
			      }
			      break;	
				 case(lastcartype):
					 return FALSE;
				 default:
					 p_tag->cartype=defaultcartype;
				   p_tag->tmcf_enc=e_pcf7936_psw;
            break;					
			 }	
        if(p_tag->cartype!=lastcartype){
           p_tag->cartype+=1;						
					if(!pcf7936_auth_cipher(p_tag)){
							 pcf7936_to_off();
					}else{
						  pcf7936_to_off();
						  p_tag->tmcf_enc=e_pcf7936_cipher;
             debug_printf("\r\ncip_cartype=%d\r\n",p_tag->cartype);						
						return TRUE;
					}
				}else{
					break;
				}
			//p_tag->cartype+=1;	
	  }			
	}
break;
default:
 return FALSE;				 
 }
}
if(p_tag->cartype>lastcartype)
	return FALSE;
  return TRUE;
}
void pcf7936_clear_tag_flag(pcf7936_tag_t *p_tag)
{
	uint32_t random=0;
	p_tag->now_state = e_power_off;
	p_tag->mode = e_pcf7936_not;	
	//if(p_tag->hitagmode==nocrty){
	//p_tag->random[0] = 0x00/*0x17*/;
	//p_tag->random[1] = 0x00/*0x17*/;
	//p_tag->random[2] = 0x00/*0x17*/;
	//p_tag->random[3] = 0x00/*0x17*/;
	/*}else if(p_tag->hitagmode==hitag2){
			p_tag->random[0] = 0x66;
	    p_tag->random[1] = 0x97;
	    p_tag->random[2] = 0x50;
	    p_tag->random[3] = 0xEB;
	}else if(p_tag->hitagmode==hitag3){
		  p_tag->random[0] = 0x66;
	    p_tag->random[1] = 0x97;
	    p_tag->random[2] = 0x50;
	    p_tag->random[3] = 0xEB;
		  p_tag->random[4] = 0xE1;
		  p_tag->random[5] = 0x93;
		  p_tag->random[6] = 0x46;
		  p_tag->random[7] = 0x00;
	}*/
	random=rand();
	p_tag->random[0]=random&0xFF;
	p_tag->random[1]=(random>>8)&0xFF;
	random=rand();
	p_tag->random[2]=(random>>8)&0xFF;
	p_tag->random[3]=random&0xFF;
}

/********************************************************
* function: taisu 46 tag copy pcf7936 all page data
*
* p_tag: pointer for store tag data
*
* return: if find tag success, return ture
********************************************************/
uint8_t pcf7936_find_tag(pcf7936_tag_t *p_tag)
{
		//uint8_t result;
	if (!pcf7936_decide_modu(p_tag))
		return FALSE;
	
	p_tag->mode = e_pcf7936_yes;
  if(!pcf7936_decide_crytmod(p_tag))
		return FALSE;

	/* ts46 blank, skey is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF */
	//p_tag->tmcf_enc = e_pcf7936_cipher;
//if(p_tag->ictype!=other_typeic){	
	/*if(p_tag->tmcf_enc == e_pcf7936_cipher){
	//p_tag->secret_key[0] = 0xFF;
	p_tag->secret_key[1] = 0xFF;
	p_tag->secret_key[2] = 0xFF;
	p_tag->secret_key[3] = 0xFF;
	p_tag->secret_key[4] = 0xFF;
	p_tag->secret_key[5] = 0xFF;*/
	//pcf7936_off_to_wait();
	//result = pcf7936_auth_cipher(p_tag);
	//pcf7936_to_off();
	//if (result) {
		//p_tag->mode = e_pcf7936_ts46_blank;
		//return TRUE;
	//}
 //}
 //}
			if (pcf7936_read_tag(p_tag, 0, 7)) {
			p_tag->now_state = e_read_all;
			p_tag->mode = e_pcf7936_cracked;
			return TRUE;
	    }

	/* ts46 last copy, skey save in eeprom */
	//p_tag->tmcf_enc = e_pcf7936_cipher;
/*
if(p_tag->tmcf_enc == e_pcf7936_cipher){
//	eep_rd_data(EEP_TS46_SKRY, p_tag->secret_key, 6);
	p_tag->a_page[2][2] = p_tag->secret_key[0];
	p_tag->a_page[2][3] = p_tag->secret_key[1];
	p_tag->a_page[1][0] = p_tag->secret_key[2];
	p_tag->a_page[1][1] = p_tag->secret_key[3];
	p_tag->a_page[1][2] = p_tag->secret_key[4];
	p_tag->a_page[1][3] = p_tag->secret_key[5];
	if (pcf7936_read_tag(p_tag, 4, 7)) {
		p_tag->now_state = e_read_all;
		p_tag->mode = e_pcf7936_cracked;
		return TRUE;
	}
 }*/
	return TRUE;
}

/********************************************************
* function: check pcf7936 tag mode and TS888 keypad interaction
*
* p_tag: pointer for store tag data
*
* return: if find tag success, return ture
********************************************************/
uint8_t pcf7936_keypad_interaction(pcf7936_tag_t *p_tag)
{
		//uint8_t key;
	//	enum {
	//	s_show_id, s_copying,
		//s_exit
	//} state;
	if (p_tag->mode == e_pcf7936_not)
		return FALSE;
	
	//LCD_Clear(BACK_COLOR);
	//LCD_Fill(10,50,lcddev.width,lcddev.height,BACK_COLOR);	state = s_show_id;
	if(p_tag->mode == e_pcf7936_yes ||p_tag->mode == e_pcf7936_cracked/*&& (state != s_exit)*/) {
		//switch (state) {
		//	case s_show_id:
			//	LCD_ShowString(30,120,200,24,24,"               ",LCD_ROW_LENGTH);
				//LCD_ShowString(10,50,200,16,16,"                  ",LCD_ROW_LENGTH);
				OLED_Clear( );
			  OLED_ShowString(0,0,str_46_tag,16,LCD_ROW_LENGTH);
				//LCD_ShowString(10,50,200,16,16, str_46_tag,LCD_ROW_LENGTH);
				//lcd_print_hex(10, 70,200,16,16, p_tag->a_page[0],4);
				//LCD_ShowString(10,90,200,16,16,"                  ",LCD_ROW_LENGTH);
			//if(p_tag->ictype==pcf7936_yes)
			 OLED_ShowString(43,0,"PCF7936",16,7);
					//LCD_ShowString(10, 90,200,16,16,"PCF7936",7);
		//	else
				//  LCD_ShowString(10, 90,200,16,16,"               ",LCD_ROW_LENGTH);
				//LCD_ShowString(30,90,200,24,24, str_exit_r);
				//LCD_ShowString(30,120,200,16,16, str_copy_c);
		//while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
			//	if (key == KEYPAD_RD_PRES)
				//	state = s_exit;
				//else
			//		state = s_copying;
				//break;
/*
			case s_copying:
				LCD_Clear(BACK_COLOR);
				LCD_ShowString(30,40,200,24,24, str_insert, 6);
				LCD_ShowString(30,70,200,24,24, str_new_46, 6);
				LCD_ShowString(30,100,200,24,24, str_exit_r, 6);
				LCD_ShowString(30,130,200,24,24, str_copy_c, 6);
				while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
				if (key == KEYPAD_RD_PRES)
					state = s_exit;
				else {
					lcd_print_copying();
					if (ts06new_init_id(p_tag->a_page[0], p_tag->tmcf_dcs))
						result = TRUE;
					else if (ts06_init_id(p_tag->a_page[0], p_tag->tmcf_dcs))
						result = TRUE;
					else
						result = FALSE;
					if (result) {
						LCD_ShowString(30,40,200,24,24, str_copy_ok, 8);
						LCD_ShowString(30,100,200,24,24, str_sniff_car_data, 14);
						while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
						state = s_exit;
					}
					else {
						LCD_ShowString(30,40,200,24,24, str_copy_ng, 8);
						LCD_ShowString(30,100,200,24,24, str_exit_r, 6);
						LCD_ShowString(30,130,200,24,24, str_copy_c, 6);
						while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
						if (key == KEYPAD_RD_PRES)
							state = s_exit;
					}
				}
				break;
*/
			//case s_exit:
		//	default :
			//	state = s_exit;
			//	break;
	//	}
	}
/*	
	while ((p_tag->mode == e_pcf7936_ts46_blank) && (state != s_exit)) {
		switch (state) {
			case s_show_id:
				//LCD_ShowString(10,50,200,16,16,"                  ",LCD_ROW_LENGTH);
				LCD_ShowString(10,50,200,16,16, str_46_tag,LCD_ROW_LENGTH);
				lcd_print_hex(10,70,200,16,16, p_tag->a_page[0], 4);
				//if(p_tag->ictype==pcf7936_yes)
					LCD_ShowString(10, 90,200,16,16,"PCF7936",7);
			 // else
				//  LCD_ShowString(10, 90,200,16,16,"               ",LCD_ROW_LENGTH);
				LCD_ShowString(10,110,200,16,16, str_blank,5);
				//LCD_ShowString(30,130,200,24,24, str_exit_r, 6);
			//	while ((key = KEY_Scan(0)) != KEYPAD_RD_PRES);
				state = s_exit;
				break;
			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}
	
	while ((p_tag->mode == e_pcf7936_cracked) && (state != s_exit)) {
		switch (state) {
			case s_show_id:
				//LCD_ShowString(10,50,200,16,16,"                  ",LCD_ROW_LENGTH);
				LCD_ShowString(10,50,200,16,16, str_46_tag,LCD_ROW_LENGTH);
					lcd_print_hex(10,70,200,16,16, p_tag->a_page[0], 4);
				//if(p_tag->ictype==pcf7936_yes)
					LCD_ShowString(10, 90,200,16,16,"PCF7936",7);
			//else
				 // LCD_ShowString(10, 90,200,16,16,"               ",LCD_ROW_LENGTH);
			 if((p_tag->a_page[3][0]&0xF0)==0x00){
			   LCD_ShowString(10,110,200,16,16, str_locked,6);
			 }else{
				 LCD_ShowString(10,110,200,16,16,"locked",6);
			 }
			  LCD_ShowString(10, 130,200,16,16,"P0:",3); 
        lcd_print_hex(33, 130,200,16,16, p_tag->a_page[0],4); 
 				LCD_ShowString(33+8*11, 130,200,16,16,"P1:",3); 
        lcd_print_hex(33+8*11+3*11, 130,200,16,16, p_tag->a_page[1],4); 
			
				LCD_ShowString(10, 150,200,16,16,"P2:",3); 
        lcd_print_hex(33, 150,200,16,16, p_tag->a_page[2],4); 
 				LCD_ShowString(33+8*11, 150,200,16,16,"P3:",3); 
        lcd_print_hex(33+8*11+3*11, 150,200,16,16, p_tag->a_page[3],4); 
			
				LCD_ShowString(10, 170,200,16,16,"P4:",3); 
        lcd_print_hex(33, 170,200,16,16, p_tag->a_page[4],4); 
 				LCD_ShowString(33+8*11, 170,200,16,16,"P5:",3); 
        lcd_print_hex(33+8*11+3*11, 170,200,16,16, p_tag->a_page[5],4); 
			
				LCD_ShowString(10, 190,200,16,16,"P6:",3); 
        lcd_print_hex(33, 190,200,16,16, p_tag->a_page[6],4); 
 				LCD_ShowString(33+8*11, 190,200,16,16,"P7:",3); 
        lcd_print_hex(33+8*11+3*11, 190,200,16,16, p_tag->a_page[7],4); */
		//while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
				//if (key == KEYPAD_RD_PRES)
					//state = s_exit;
				/*else
					state = s_copying;
				break;

			case s_copying:
				LCD_Clear(BACK_COLOR);
				LCD_ShowString(30,40,200,24,24, str_insert, 6);
				LCD_ShowString(30,70,200,24,24,str_new_46, 6);
				LCD_ShowString(30,100,200,24,24, str_exit_r, 6);
				LCD_ShowString(30,130,200,24,24, str_copy_c, 6);
				while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
				if (key == KEYPAD_RD_PRES)
					state = s_exit;
				else {
					lcd_print_copying();
					if (tag_copy_pcf7936(&ts06_tag1, &pcf7936_tag_temp, p_tag))
						LCD_ShowString(30,40,200,24,24, str_copy_ok, 8);
					else
						LCD_ShowString(30,40,200,24,24, str_copy_ng, 8);
					LCD_ShowString(30,100,200,24,24, str_exit_r, 6);
					LCD_ShowString(30,130,200,24,24, str_copy_c, 6);
					while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
					if (key == KEYPAD_RD_PRES)
						state = s_exit;
					else
						state = s_copying;
				}*/
		/*		break;

			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}*/
	
	return TRUE;
}

void pcf7936_fast_read_init(void)
{
	LCD_Clear(BACK_COLOR);
	//led_on();
	delay_us(500*1000);
	//led_off();
	
	LCD_ShowString(30,40,200,24,24, "FastRead46 V1.1",15);
}

void pcf7936_fast_read(pcf7936_tag_t *p_tag)
{
	uint8_t a_uart[15], i;
	
	enum {
		s_read_bp, s_read_mc,
		s_show_id, s_exit
	} state;
	
	state = s_read_bp;
	while (state != s_exit) {
		switch (state) {
			case s_read_bp:
				p_tag->tmcf_dcs = e_modu_bp;
				pcf7936_off_to_wait();
				if (pcf7936_auth_half(p_tag))
					state = s_show_id;
				else
					state = s_read_mc;
				pcf7936_to_off();
				break;
			case s_read_mc:
				p_tag->tmcf_dcs = e_modu_mc;
				pcf7936_off_to_wait();
				if (pcf7936_auth_half(p_tag))
					state = s_show_id;
				else
					state = s_exit;
				pcf7936_to_off();
				break;
			case s_show_id:
				//led_on();
				a_uart[0] = 0xA5;
				a_uart[1] = 0xFF;
				a_uart[2] = 3+4;
				a_uart[3] = 0x46;
				a_uart[4] = 4;
				copy_bytes(&a_uart[5], p_tag->a_page[0], 4);
				//a_uart[9] = calculate_sum(a_uart, 9);
				//uart_send_str(a_uart, 10);
				lcd_print_hex(30, 40,200,24,24, p_tag->a_page[0], 4);

				i = 0;
				while (i < 2) {
					pcf7936_off_to_wait();
					if (pcf7936_auth_half(p_tag))
						i = 0;
					else
						i++;
					pcf7936_to_off();
				}
				
			//	led_off();
				lcd_print_blank(1);
				state = s_exit;
				break;
			default :
				state = s_exit;
				break;
		}
	}
}

/********************************************************
* function: ts06new(taisu new 06 tag, OuLongZhen design this tag)
*
* page: witch page need read, 0-15
* p_rd: pointer for save page data
*
* return: if read success, return ture
********************************************************/
uint8_t ts06new_test_read(uint8_t page, uint8_t *p_rd)
{
		uint8_t result;
	uint8_t a_cmd[2], a_rd[5];
	if (page > 15) {
		return FALSE;
	}

	a_cmd[0] = 0x06 | (page<<3);
	if (calculate_msbit_parity(&a_cmd[0], 8))
		a_cmd[0] |= 0x80;
	a_cmd[1] = ~a_cmd[0];

	result = FALSE;
	pcf7936_off_to_wait();
	//pcf7991_write_tag_n(0);
	pcf7991_write_tag_n(0x07);
	pcf7936_wr_msbits(&a_cmd[0], 7, FALSE);
	pcf7936_wr_msbits(&a_cmd[1], 7, TRUE);
	pcf7991_set_page(0x00, g_pcf7991_p0_pcf7936);
	pcf7991_read_tag();
	delay_us(PCF7936_T_WAIT_TR);
	if (pcf7991_decode_miss_low()) {
		result = pcf7991_decode_mc(32, a_rd, 37, e_dir_msbit);
	}
	if (!result)
		delay_us(10*1000);
	else if ((a_rd[0]&0xF0) == 0xF0) {
		copy_msbits(p_rd, 0, a_rd, 5, 32);
	}
	else
		result = FALSE;
	pcf7936_to_off();

	return result;
}

/********************************************************
* function: ts06new(taisu new 06 tag, OuLongZhen design this tag)
*
* page: witch page need write, 0-15
* p_rd: pointer for write page data
*
* return: if write success, return ture
********************************************************/
uint8_t ts06new_test_write(uint8_t page, const uint8_t *p_wr)
{
		uint8_t result;
	uint8_t a_cmd[2], a_rd[5], a_response[2];
	if (page > 15) {
		return FALSE;
	}

	a_cmd[0] = 0x04 | (page<<3);
	if (calculate_msbit_parity(&a_cmd[0], 8))
		a_cmd[0] |= 0x80;
	a_cmd[1] = ~a_cmd[0];

	/* clear, in order to compare later */
	a_response[0] = 0x00;
	a_response[1] = 0x00;

	result = FALSE;
	pcf7936_off_to_wait();
	//pcf7991_write_tag_n(0);
	pcf7991_write_tag_n(0x07);
	pcf7936_wr_msbits(&a_cmd[0], 7, FALSE);
	pcf7936_wr_msbits(&a_cmd[1], 7, TRUE);
	pcf7991_set_page(0x00, g_pcf7991_p0_pcf7936);
	pcf7991_read_tag();
	delay_us(PCF7936_T_WAIT_TR);
	if (pcf7991_decode_miss_low()) {
		result = pcf7991_decode_mc(32, a_rd, 19, e_dir_msbit);
	}

	if (!result)
		delay_us(8*1000);
	else if ((a_rd[0]&0xF8) == 0xF8) {
		copy_msbits(a_response, 0, a_rd, 5, 14);
	}
	else
		result = FALSE;

	/* now don't compare response, because tag performance not very good */
	delay_us(PCF7936_T_WAIT_BS);
	//pcf7991_write_tag_n(0);
	pcf7991_write_tag_n(0x07);
	pcf7936_wr_msbits(p_wr, 32, TRUE);
	delay_us(PCF7936_T_PROG*1000);
	pcf7936_to_off();

	/* check, read back page and compare */
	if (!ts06new_test_read(page, a_rd))
		result = FALSE;
	else if(!equal_bytes(p_wr, a_rd, 4))
		result = FALSE;
	else
		result = TRUE;

	return result;
}

uint8_t ts06new_init_id(uint8_t *p_id, uint8_t code)
{
	uint8_t result;
	uint8_t a_page3[4], a_page15[5], a_rd[5];

	if (code == e_modu_bp)
		a_page3[0] = 0x0F;
	else if (code == e_modu_mc)
		a_page3[0] = 0x0E;
	else
		return FALSE;
	a_page3[1] = 0x00;
	a_page3[2] = 0x00;
	a_page3[3] = 0x00;
	a_page15[0] = 0xBC;		// collect set as 0xBC, normal set as 0x00
	a_page15[1] = 0x00;
	a_page15[2] = 0x00;
	a_page15[3] = 0x00;
	a_page15[4] = 0x00;
	
	result = FALSE;
	/* read first in order to check tag inside */
	if (!ts06new_test_read(15, &a_rd[0]))
		;
	/* start write tag */
	else if (!ts06new_test_write(15, a_page15))
		;
	else if (!ts06new_test_write(8, &a_page15[1]))
		;
	else if (!ts06new_test_write(9, &a_page15[1]))
		;
	else if (!ts06new_test_write(10, &a_page15[1]))
		;
	else if (!ts06new_test_write(11, &a_page15[1]))
		;
	else if (!ts06new_test_write(12, &a_page15[1]))
		;
	else if (!ts06new_test_write(13, &a_page15[1]))
		;
	else if (!ts06new_test_write(3, a_page3))
		;
	else if (!ts06new_test_write(0, p_id))
		;
	else
		result = TRUE;
	
	return result;
}

uint8_t ts06new_copy_pcf7936(const pcf7936_tag_t *p_tag_rd)
{
	static const uint8_t a_page15[4] = {0x00, 0x00, 0x00, 0x00};
	
	if (!ts06new_test_write(15, a_page15))
		;
	else if (!ts06new_test_write(4, p_tag_rd->a_page[4]))
		;
	else if (!ts06new_test_write(5, p_tag_rd->a_page[5]))
		;
	else if (!ts06new_test_write(6, p_tag_rd->a_page[6]))
		;
	else if (!ts06new_test_write(7, p_tag_rd->a_page[7]))
		;
	else if (!ts06new_test_write(0, p_tag_rd->a_page[0]))
		;
	else if (!ts06new_test_write(1, p_tag_rd->a_page[1]))
		;
	else if (!ts06new_test_write(2, p_tag_rd->a_page[2]))
		;
	else if (!ts06new_test_write(3, p_tag_rd->a_page[3]))
		;
	else
		return TRUE;

	return FALSE;
}

ts06new_tag_t ts06new_tag1;

/********************************************************
* p_tag:		pointer for store data as taisu new 06 tag
* p_tag_ori:	pointer for store data as pcf7936 tag
*
* return: if find success, return ture
********************************************************/
uint8_t ts06new_find_tag(ts06new_tag_t *p_tag, pcf7936_tag_t *p_tag_ori)
{
	uint8_t i, j;
	
	p_tag->collect = 0;
	p_tag->mode = FALSE;
	/* read tag ID as pcf7936 command */
	if (!pcf7936_decide_modu(p_tag_ori))
		;
	/* read tag ID as taisu inside command */
	else if (!ts06new_test_read(0, p_tag->a_id))
		;
	else if (!ts06new_test_read(8, p_tag->a_sign[0]))
		;
	else if (!ts06new_test_read(9, p_tag->a_sign[1]))
		;
	else if (!ts06new_test_read(10, p_tag->a_sign[2]))
		;
	else if (!ts06new_test_read(11, p_tag->a_sign[3]))
		;
	else if (!ts06new_test_read(12, p_tag->a_sign[4]))
		;
	else if (!ts06new_test_read(13, p_tag->a_sign[5]))
		;
	/* check, compare two method read result */
	else if (!equal_bytes(p_tag->a_id, p_tag_ori->a_page[0], 4))
		;
	else {
		p_tag->mode = TRUE;
		p_tag->tmcf_dcs = p_tag_ori->tmcf_dcs;
		for (i = 0; i < 6; i+=2) {
			if (!equal_byte(p_tag->a_sign[i], 0x00, 8))
				p_tag->collect += 1;
		}

		/* compatible ts06 data */
		for (i = 0; i < 6; i++)
			for (j = 0; j < 4; j++)
				p_tag->a_sign[i][j] = reverse_byte(p_tag->a_sign[i][j]);
	}

	return p_tag->mode;
}

uint8_t ts06new_keypad_interaction(ts06new_tag_t *p_tag)
{
		uint8_t key, i_group;
	uint8_t a_display[LCD_ROW_LENGTH];
	enum {
		s_waiting, 
		s_crack, s_show_skey, s_crack_copy,
		s_exit
	} state;
	if (p_tag->mode == FALSE)
		return FALSE;


	state = s_waiting;
	while (state != s_exit) {
		switch (state) {
			case s_waiting:
				/* taisu new 06 tag just have 3 groups signature */
				a_display[0] = '(';
				a_display[1] = '0' + (p_tag->collect / 10);
				a_display[2] = '0' + (p_tag->collect % 10);
				a_display[3] = '/';
				a_display[4] = '0';
				a_display[5] = '3';
				a_display[6] = ')';
				a_display[7] = ' ';
				LCD_Clear(BACK_COLOR);
				LCD_ShowString(30,40,200,24,24, str_46_tag, 6);
				lcd_print_hex(30,70,200,24,24, p_tag->a_id, 4);
				delay_us(T_DISPLAY*1000);
				LCD_ShowString(30,100,200,24,24, a_display, 8);
				//LCD_ShowString(30,130,200,24,24, str_exit_c, 6);
				if (p_tag->collect > 0)
					LCD_ShowString(30,130,200,24,24, str_crack_c, 7);
				while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
				if (key == KEYPAD_RD_PRES)
					state = s_exit;
				else if (p_tag->collect > 0)
					state = s_crack;
				else
					state = s_exit;
				break;

			case s_crack:
				LCD_Clear(BACK_COLOR);
				LCD_ShowString(30,40,200,24,24, str_insert_ori_key, 14);
				//LCD_ShowString(30,100,200,24,24, str_exit_r, 6);
				LCD_ShowString(30,130,200,24,24, str_crack_c, 7);
				while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
				if (key == KEYPAD_RD_PRES)
					state = s_exit;
				else {
					copy_bytes(sk_uid, p_tag->a_id, 4);
					a_display[0] = '/';
					a_display[1] = ' ';
					a_display[2] = '0';
					a_display[3] = '0';
					a_display[4] = '3';
					for (i_group = 0; i_group < 6; i_group += 2) {
						LCD_Clear(BACK_COLOR);
						LCD_ShowString(30,40,200,24,24, str_reading, 7);
						//lcd_print_bcd(0, 1, i_group/2+1);
						LCD_ShowString(30,100,200,24,24, a_display, 5);
						/* !!!!!!!!!!!!!!! 46 crack !!!!!!!!!!!!!!! */
						if (Fccrack46(p_tag->tmcf_dcs, p_tag->a_sign[i_group], p_tag->a_keys))
							break;
					}
					pcf7936_tag1.tmcf_dcs = p_tag->tmcf_dcs;
					pcf7936_tag1.tmcf_enc = e_pcf7936_cipher;
					copy_bytes(pcf7936_tag1.secret_key, p_tag->a_keys, 6);
					LCD_Clear(BACK_COLOR);
					if (i_group >= 6) {
						state = s_exit;
					}
					else if (!pcf7936_read_tag(&pcf7936_tag1, 3, 7)) {
						state = s_exit;
					}
					else {
						copy_bytes(&(pcf7936_tag1.a_page[2][2]), pcf7936_tag1.secret_key, 2);
						copy_bytes(pcf7936_tag1.a_page[1], &(pcf7936_tag1.secret_key[2]), 4);
						LCD_ShowString(30,40,200,24,24, str_crack_ok, 8);
						delay_us(T_DISPLAY*1000);
						state = s_show_skey;
					}

					if (state == s_exit) {
						LCD_ShowString(30,40,200,24,24, str_crack_no, 16);
						LCD_ShowString(30,100,200,24,24, &str_crack_no[16], 16);
						while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
					}
				}
				break;
			case s_show_skey:
				LCD_Clear(BACK_COLOR);
				lcd_print_hex(30, 40,200,24,24, pcf7936_tag1.secret_key, 6);
				//LCD_ShowString(30,100,200,24,24, str_exit_r, 6);
				LCD_ShowString(30,130,200,24,24, str_copy_c, 6);
				while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
				if (key == KEYPAD_RD_PRES)
					state = s_exit;
				else
					state = s_crack_copy;
				break;
			case s_crack_copy:
				LCD_Clear(BACK_COLOR);
				LCD_ShowString(30,40,200,24,24, str_insert, 6);
				LCD_ShowString(30,70,200,24,24, str_new_46, 4);
				LCD_ShowString(30,100,200,24,24, str_exit_r, 6);
				LCD_ShowString(30,130,200,24,24, str_copy_c, 6);
				while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
				if (key == KEYPAD_RD_PRES)
					state = s_exit;
				else {
					lcd_print_copying();
					if (tag_copy_pcf7936(&ts06_tag1, &pcf7936_tag_temp, &pcf7936_tag1))
						LCD_ShowString(30,40,200,24,24, str_copy_ok, 8);
					else
						LCD_ShowString(30,40,200,24,24, str_copy_ng, 8);
					LCD_ShowString(30,100,200,24,24, str_exit_r, 6);
					LCD_ShowString(30,130,200,24,24, str_copy_c, 6);
					while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
					if (key == KEYPAD_RD_PRES)
						state = s_exit;
					else
						state = s_crack_copy;
				}
				break;
			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}
	
	return TRUE;
}

/********************************************************
* ts46 can copy pcf7936
* ts06 can copy pcf7936
* ts06 new can copy pcf7936
*
* return: if copy success, return ture
********************************************************/
uint8_t tag_copy_pcf7936(ts06_tag_t *p_ts06, pcf7936_tag_t *p_ts46, pcf7936_tag_t *p_tag_rd)
{
	uint8_t result/*, a_skey[6]*/;
	/* in order to save last copy skey */
	//a_skey[0] = p_tag_rd->a_page[2][2];
	//a_skey[1] = p_tag_rd->a_page[2][3];
	//a_skey[2] = p_tag_rd->a_page[1][0];
	//a_skey[3] = p_tag_rd->a_page[1][1];
	//a_skey[4] = p_tag_rd->a_page[1][2];
	//a_skey[5] = p_tag_rd->a_page[1][3];

	/* always cipher mode, compatible with pcf7941 */
	p_tag_rd->a_page[3][0] |= 0x0E;
	
	if (ts06_copy_pcf7936(p_ts06, p_tag_rd))
		result = TRUE;
	else if (ts06new_copy_pcf7936(p_tag_rd))
		result = TRUE;
	else if (ts46_copy_pcf7936(p_ts46, p_tag_rd))
		result = TRUE;
	else
		result = FALSE;

	/* SKEY save in EEPROM */
	if (result) {
		//eep_wr_data(EEP_TS46_SKRY, a_skey, 6);
	}

	return result;
}


#define PCF7936_ID_EEP_ADDR		(0xFF2)
#define PCF7936_TYPE_NXP		(1)
#define PCF7936_TYPE_TS			(!PCF7936_TYPE_NXP)

void ts06new_init_tag_init(void)
{
	uint8_t i=0;
	uint8_t seed=0;
	
//	eep_rd_data(PCF7936_ID_EEP_ADDR, &seed, 1);
	srand((seed<<8) | seed);
	seed++;

	//for (i = 10; i > 0; i--)
	//	if (eep_wr_data(PCF7936_ID_EEP_ADDR, &seed, 1))
		//	break;

	LCD_Clear(BACK_COLOR);
	if (i > 0) {
		//led_on();
		//delay_us(500*1000);
		//led_off();

		#if PCF7936_TYPE_NXP
		LCD_ShowString(30,40,200,24,24, "Init06NXP V2.0N", 15);
		#else
		LCD_ShowString(30,40,200,24,24, "Init06TS  V2.0T", 15);
		#endif
	}
	else {
		LCD_ShowString(30,40,200,24,24, "Mega128 EEP Not", 15);
		while(1);
	}
}
void ts06_pcf7936typeinit_tag(pcf7936_tag_t *p_tag)
{
	//#if PCF7936_TYPE_NXP
	//static  uint8_t a_page1[4] = {0x4D, 0x49, 0x4B, 0x52};
	//static  uint8_t a_page2[4] = {0x96, 0xF8, 0x4F, 0x4E};
	//static const uint8_t a_page3[4] = {0x06, 0xAA, 0x48, 0x54};
	
	//static const uint8_t a_page4[4] = {0x46, 0x5F, 0x4F, 0x4B};
	//static const uint8_t a_page5[4] = {0x55, 0x55, 0x55, 0x55};
	//static const uint8_t a_page6[4] = {0xAA, 0xAA, 0xAA, 0xAA};
	//static const uint8_t a_page7[4] = {0xFF, 0xFF, 0xFF, 0xFF};
	//#endif
	//static const uint8_t a_page15[4] = {0x00, 0x00, 0x00, 0x00};

	//static uint8_t a_uart[40];
	//uint8_t a_page0[4], i;
	//uint16_t random;
//	uint8_t (* pf_auth)(pcf7936_tag_t *);	
	//	p_tag->ictype=ictype;
	 // p_tag->hitagmode=crotymode;
	pcf7936_clear_tag_flag(p_tag);
			//#if PCF7936_TYPE_NXP
				//p_tag->tmcf_enc = e_pcf7936_psw;
				//copy_bytes(p_tag->a_page[1], a_page1, 4);
			//	pf_auth = pcf7936_auth_psw;
			/*if(p_tag->hitagmode==hitag2){
       a_page1[0] =0x4D;
			 a_page1[1] =0x49;
			 a_page1[2] =0x4B;
			 a_page1[3] =0x52;
       //a_page2[0] = 0x96;
			 //a_page2[1] = 0xF8;
			 a_page2[2] = 0x4F;
			 a_page2[3] =0x4E;
				copy_bytes(p_tag->secret_key, &a_page2[2], 2);
				copy_bytes(&(p_tag->secret_key[2]), a_page1, 4);
					}else if(p_tag->hitagmode==hitag3){
						p_tag->secret_key[0]=0x66;
						p_tag->secret_key[1]=0x66;
						p_tag->secret_key[2]=0x11;
						p_tag->secret_key[3]=0x11;
						p_tag->secret_key[4]=0x22;
						p_tag->secret_key[5]=0x22;
						p_tag->secret_key[6]=0x33;
						p_tag->secret_key[7]=0x33;
						p_tag->secret_key[8]=0x44;
						p_tag->secret_key[9]=0x44;
						p_tag->secret_key[10]=0x55;
						p_tag->secret_key[11]=0x55;
					}   
	  // a_page3[4] = {0x0E, 0xAA, 0x48, 0x54};
				p_tag->tmcf_enc = e_pcf7936_cipher;
				//pf_auth = pcf7936_auth_cipher;
				}*/
		//	#endif
}
void ts06new_init_tag(pcf7936_tag_t *p_tag)
{
	#if PCF7936_TYPE_NXP
	static const uint8_t a_page1[4] = {0x4D, 0x49, 0x4B, 0x52};
	//static const uint8_t a_page2[4] = {0x96, 0xF8, 0x4F, 0x4E};
	//static const uint8_t a_page3[4] = {0x06, 0xAA, 0x48, 0x54};
	
	//static const uint8_t a_page4[4] = {0x46, 0x5F, 0x4F, 0x4B};
	//static const uint8_t a_page5[4] = {0x55, 0x55, 0x55, 0x55};
	//static const uint8_t a_page6[4] = {0xAA, 0xAA, 0xAA, 0xAA};
	//static const uint8_t a_page7[4] = {0xFF, 0xFF, 0xFF, 0xFF};

	#else
	static const uint8_t a_page1[4] = {0xFF, 0xFF, 0xFF, 0xFF};
	static const uint8_t a_page2[4] = {0x96, 0xF8, 0xFF, 0xFF};
	//static const uint8_t a_page3[4] = {0x0E, 0xAA, 0x48, 0x54};
	#endif
	//static const uint8_t a_page15[4] = {0x00, 0x00, 0x00, 0x00};

	//static uint8_t a_uart[40];
	//uint8_t a_page0[4], i;
	//uint16_t random;
//	uint8_t (* pf_auth)(pcf7936_tag_t *);
		
	enum {
		s_auth,
		s_init_p15, s_init_p4, s_init_p5, s_init_p6, s_init_p7, 
		s_init_p0, s_init_p1, s_init_p2, s_init_p3,
		s_ok, s_exit
	} state;

	pcf7936_clear_tag_flag(p_tag);
	state = s_auth;
	while (state != s_exit) {
		switch (state) {
			case s_auth:
				p_tag->tmcf_dcs = e_modu_mc;
			#if PCF7936_TYPE_NXP

				p_tag->tmcf_enc = e_pcf7936_psw;
				copy_bytes(p_tag->a_page[1], a_page1, 4);
			//	pf_auth = pcf7936_auth_psw;

			#else
				p_tag->tmcf_enc = e_pcf7936_cipher;
				copy_bytes(p_tag->secret_key, &a_page2[2], 2);
				copy_bytes(&(p_tag->secret_key[2]), a_page1, 4);
				//pf_auth = pcf7936_auth_cipher;
				}
			#endif
				//pcf7936_off_to_wait();
				//if (!pf_auth(p_tag))
			//		state = s_init_p15;
				//else if (!equal_bytes(p_tag->a_page[3], a_page3, 4))
			//		state = s_init_p15;
			//	else
					state = s_ok;
			//	pcf7936_to_off();
				break;
/*
			case s_init_p15:
				if (ts06new_test_write(15, a_page15))
					state = s_init_p4;
				else if (ts06new_test_write(15, a_page15))
					state = s_init_p4;
				else
					state = s_exit;
				break;

			case s_init_p4:
			#if PCF7936_TYPE_NXP
				if (ts06new_test_write(4, a_page4))
					state = s_init_p5;
				else if (ts06new_test_write(4, a_page4))
					state = s_init_p5;
				else
					state = s_exit;
			#else
				state = s_init_p5;
			#endif
				break;
			case s_init_p5:
			#if PCF7936_TYPE_NXP
				if (ts06new_test_write(5, a_page5))
					state = s_init_p6;
				else if (ts06new_test_write(5, a_page5))
					state = s_init_p6;
				else
					state = s_exit;
			#else
				state = s_init_p6;
			#endif
				break;
			case s_init_p6:
			#if PCF7936_TYPE_NXP
				if (ts06new_test_write(6, a_page6))
					state = s_init_p7;
				else if (ts06new_test_write(6, a_page6))
					state = s_init_p7;
				else
					state = s_exit;
			#else
				state = s_init_p7;
			#endif
				break;
			case s_init_p7:
			#if PCF7936_TYPE_NXP
				if (ts06new_test_write(7, a_page7))
					state = s_init_p0;
				else if (ts06new_test_write(7, a_page7))
					state = s_init_p0;
				else
					state = s_exit;
			#else
				state = s_init_p0;
			#endif
				break;
				
			case s_init_p0:
				random = rand();
				a_page0[0] = (uint8_t)(random>>8);
				a_page0[1] = a_page0[0] + 0x11;
				a_page0[2] = (uint8_t)random;
				a_page0[3] = (a_page0[2] & 0x0F) | 0x10; // see pcf7936 datasheet EEPROM Content Upon Delivery
				if (ts06new_test_write(0, a_page0))
					state = s_init_p1;
				else if (ts06new_test_write(0, a_page0))
					state = s_init_p1;
				else
					state = s_exit;
				break;
			case s_init_p1:
				if (ts06new_test_write(1, a_page1))
					state = s_init_p2;
				else if (ts06new_test_write(1, a_page1))
					state = s_init_p2;
				else
					state = s_exit;
				break;
			case s_init_p2:
				if (ts06new_test_write(2, a_page2))
					state = s_init_p3;
				else if (ts06new_test_write(2, a_page2))
					state = s_init_p3;
				else
					state = s_exit;
				break;
			case s_init_p3:
				if (ts06new_test_write(3, a_page3))
					state = s_auth;
				else if (ts06new_test_write(3, a_page3))
					state = s_auth;
				else
					state = s_exit;
				break;
	*/			
			case s_ok:
				//LCD_ShowString(10,50,200,16,16,"                  ",LCD_ROW_LENGTH);
				//lcd_print_hex(10, 50,200,16,16, p_tag->a_page[0], 4);
/*
				i = 0;
				while (i < 3) {
					pcf7936_off_to_wait();
					if (pcf7936_auth_half(p_tag))
						i = 0;
					else
						i++;
					pcf7936_to_off();
				}
			//	led_off();
				lcd_print_blank(1);*/
				state = s_exit;
				break;
				
			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}
}

/********************************************************
* function: ts46(taisu 46 tag)
*
* page: witch page need read, 0-15
* p_rd: pointer for save page data
*
* return: if read success, return ture
********************************************************/
uint8_t ts46_test_read(uint8_t page, uint8_t *p_rd)
{
		uint8_t result;
	uint8_t a_cmd[2], a_rd[5];
	if (page > 15) {
		return FALSE;
	}


	a_cmd[0] = 0xC0 + (page<<1);
	a_cmd[1] = 0x00;

	result = FALSE;
	pcf7936_off_to_wait();
	//pcf7991_write_tag_n(0);
	pcf7991_write_tag_n(0x07);
	pcf7936_wr_msbits(a_cmd, 10, TRUE);
	pcf7991_set_page(0x00, g_pcf7991_p0_pcf7936);
	pcf7991_read_tag();
	delay_us(PCF7936_T_WAIT_TR);
	if (pcf7991_decode_miss_low()) {
		result = pcf7991_decode_mc(32, a_rd, 37, e_dir_msbit);
	}
	if (!result)
		delay_us(10*1000);
	pcf7936_to_off();

	if (!result)
		;
	else if ((a_rd[0]&0xF8) == 0xF8) {
		copy_msbits(p_rd, 0, a_rd, 5, 32);
	}
	else
		result = FALSE;

	return result;
}

/********************************************************
* function: ts46(taisu 46 tag)
*
* page: witch page need write, 0-15
* p_rd: pointer for write page data
*
* return: if write success, return ture
********************************************************/
uint8_t ts46_test_write(uint8_t page, const uint8_t *p_wr, uint8_t all_flag)
{
		uint8_t result;
	uint8_t a_cmd[2], a_rd[5], a_response[2];
	if (page > 15) {
		return FALSE;
	}


	a_cmd[0] = 0x80 + (page<<1);
	if (all_flag)
		a_cmd[0] |= 0x21;
	a_cmd[1] = 0x00;
	a_response[0] = 0x00;
	a_response[1] = 0x00;

	result = FALSE;
	pcf7936_off_to_wait();
	//pcf7991_write_tag_n(0);
	pcf7991_write_tag_n(0x07);
	pcf7936_wr_msbits(a_cmd, 10, TRUE);
	pcf7991_set_page(0x00, g_pcf7991_p0_pcf7936);
	pcf7991_read_tag();
	delay_us(PCF7936_T_WAIT_TR);
	if (pcf7991_decode_miss_low()) {
		result = pcf7991_decode_mc(32, a_rd, 15, e_dir_msbit);
	}

	if (!result)
		;
	else if ((a_rd[0]&0xF8) == 0xF8) {
		copy_msbits(a_response, 0, a_rd, 5, 10);
		result = equal_bytes(a_cmd, a_response, 2);
	}
	else
		result = FALSE;

	#if 1
	if (result) {
		delay_us(PCF7936_T_WAIT_BS);
		//pcf7991_write_tag_n(0);
		pcf7991_write_tag_n(0x07);
		pcf7936_wr_msbits(p_wr, 32, TRUE);
		delay_us(PCF7936_T_PROG*1000);
	}
	else
		delay_us(8*1000);
	pcf7936_to_off();

	/* check, read back and compare */
	if (result) {
		if (page == 15)
			;
		else if (!ts46_test_read(page, a_rd))
			result = FALSE;
		else if(!equal_bytes(p_wr, a_rd, 4))
			result = FALSE;
	}
	#else
	/* for future use */
	if (!result)
		delay_us(7*1000);
	delay_us(PCF7936_T_WAIT_BS);
	//pcf7991_write_tag_n(0);
	pcf7991_write_tag_n(0x07);
	pcf7936_wr_msbits(p_wr, 32, TRUE);
	delay_us(PCF7936_T_PROG*1000);
	pcf7936_to_off();

	/* check */
	if (page == 15)
		;
	else if (!ts46_test_read(page, a_rd))
		result = FALSE;
	else if(!equal_bytes(p_wr, a_rd, 4))
		result = FALSE;
	#endif

	return result;
}

void ts46_init_tag_init(void)
{
	uint8_t i=0;
	uint8_t seed=0;
	
//	eep_rd_data(PCF7936_ID_EEP_ADDR, &seed, 1);
	srand((seed<<8) | seed);
	seed++;

	//for (i = 10; i > 0; i--)
	//	if (eep_wr_data(PCF7936_ID_EEP_ADDR, &seed, 1))
	//		break;

	LCD_Clear(BACK_COLOR);
	if (i > 0) {
		//led_on();
		//delay_us(500*1000);
		//led_off();

		#if PCF7936_TYPE_NXP
		LCD_ShowString(30,40,200,24,24, "TS46InitN V2.0N", 15);
		#else
		LCD_ShowString(30,40,200,24,24, "TS46InitT V2.0T", 15);
		#endif
	}
	else {
		LCD_ShowString(30,40,200,24,24, "Mega128 EEP Not", 15);
		while(1);
	}
}

void ts46_init_tag(pcf7936_tag_t *p_tag)
{
	static const uint8_t a_page3_pre[4] = {0x06, 0x00, 0x00, 0x00};
	static const uint8_t a_page1_pre[4] = {0x00, 0x00, 0x00, 0x00};
	static const uint8_t a_page15[4] = {0x96, 0x0F, 0x00, 0x00};
	#if PCF7936_TYPE_NXP
	static const uint8_t a_page1[4] = {0x4D, 0x49, 0x4B, 0x52};
	static const uint8_t a_page2[4] = {0x96, 0xF8, 0x4F, 0x4E};
	static const uint8_t a_page3[4] = {0x06, 0xAA, 0x48, 0x54};
	static const uint8_t a_page4_5_6_7[4][4] = {
		{0x46, 0x5F, 0x4F, 0x4B},
		{0x55, 0x55, 0x55, 0x55},
		{0xAA, 0xAA, 0xAA, 0xAA},
		{0xFF, 0xFF, 0xFF, 0xFF}
		};
	#else
	static const uint8_t a_page1[4] = {0xFF, 0xFF, 0xFF, 0xFF};
	static const uint8_t a_page2[4] = {0x96, 0xF8, 0xFF, 0xFF};
	static const uint8_t a_page3[4] = {0x0E, 0xAA, 0x48, 0x54};
	#endif

	static uint8_t a_uart[40];
	uint8_t i, a_page0[4], a_rd[4];
	uint16_t random;
	uint8_t (* pf_auth)(pcf7936_tag_t *);
		
	enum {
		s_check_test_mode, s_read_tag,
		s_init_pre_p3_1_15, 
		s_init_p2, s_init_p0, s_init_p4_5_6_7, s_init_p1, s_init_p3, 
		s_init_ok, s_exit
	} state;

	pcf7936_clear_tag_flag(p_tag);
	p_tag->tmcf_dcs = e_modu_mc;
	p_tag->tmcf_enc = e_pcf7936_psw;
	copy_bytes(p_tag->a_page[1], a_page1_pre, 4);

	state = s_check_test_mode;
	while (state != s_exit) {
		switch (state) {
			case s_check_test_mode:
				if (ts46_test_read(15, a_rd))
					state = s_init_pre_p3_1_15;
				else
					state = s_read_tag;
				break;

			case s_read_tag:
			#if PCF7936_TYPE_NXP
				p_tag->tmcf_enc = e_pcf7936_psw;
				pf_auth = pcf7936_auth_psw;
			#else
				p_tag->tmcf_enc = e_pcf7936_cipher;
				pf_auth = pcf7936_auth_cipher;
			#endif
				copy_bytes(&(p_tag->secret_key[0]), &a_page2[2], 2);
				copy_bytes(&(p_tag->secret_key[2]), a_page1, 4);
				pcf7936_off_to_wait();
				if (!pf_auth(p_tag))
					state = s_exit;
				else if (!equal_bytes(p_tag->a_page[3], a_page3, 4))
					state = s_exit;
				else
					state = s_init_ok;
				pcf7936_to_off();
				break;

			case s_init_pre_p3_1_15:
				if (!ts46_test_write(3, a_page3_pre, FALSE))
					state = s_exit;
				else if (!ts46_test_write(1, a_page1_pre, FALSE))
					state = s_exit;
				else if (!ts46_test_write(15, a_page15, FALSE))
					state = s_exit;
				else
					state = s_init_p2;
				break;

			case s_init_p2:
				state = s_exit;
				pcf7936_off_to_wait();
				if (pcf7936_auth_psw(p_tag)) {
					delay_us(PCF7936_T_WAIT_BS);
					if (pcf7936_write_page(2, p_tag, a_page2)) {
						state = s_init_p0;
						p_tag->secret_key[0] = a_page2[2];
						p_tag->secret_key[1] = a_page2[3];
					}
				}
				pcf7936_to_off();
				break;

			case s_init_p0:
				random = rand();
				a_page0[0] = (uint8_t)(random>>8);
				a_page0[1] = a_page0[0] ^ 0xA5;
				a_page0[2] = (uint8_t)random;
				a_page0[3] = (a_page0[2] & 0x0F) | 0x10; // see pcf7936 datasheet EEPROM Content Upon Delivery

				state = s_exit;
				pcf7936_off_to_wait();
				if (pcf7936_auth_psw(p_tag)) {
					delay_us(PCF7936_T_WAIT_BS);
					if (pcf7936_write_page(0, p_tag, a_page0)) {
						state = s_init_p4_5_6_7;
					}
				}
				pcf7936_to_off();
				break;

			case s_init_p4_5_6_7:
			#if PCF7936_TYPE_NXP
				state = s_exit;
				pcf7936_off_to_wait();
				if (pcf7936_auth_psw(p_tag)) {
					for (i = 4; i < 8; i++) {
						delay_us(PCF7936_T_WAIT_BS);
						if (!pcf7936_write_page(i, p_tag, a_page4_5_6_7[i-4]))
							break;
					}
					if (i >= 8)
						state = s_init_p1;
				}
				pcf7936_to_off();
			#else
				state = s_init_p1;
			#endif
				break;

			case s_init_p1:
				state = s_exit;
				pcf7936_off_to_wait();
				if (pcf7936_auth_psw(p_tag)) {
					delay_us(PCF7936_T_WAIT_BS);
					if (pcf7936_write_page(1, p_tag, a_page1)) {
						state = s_init_p3;
						copy_bytes(p_tag->a_page[1], a_page1, 4);
						copy_bytes(&(p_tag->secret_key[2]), a_page1, 4);
					}
				}
				pcf7936_to_off();
				break;

			case s_init_p3:
				state = s_exit;
				pcf7936_off_to_wait();
				if (pcf7936_auth_psw(p_tag)) {
					delay_us(PCF7936_T_WAIT_BS);
					if (pcf7936_write_page(3, p_tag, a_page3)) {
						state = s_read_tag;
					}
				}
				pcf7936_to_off();
				break;

			case s_init_ok:
			//	led_on();

				a_uart[0] = 0xA5;
				a_uart[1] = 0xFF;
				a_uart[2] = 35;
				a_uart[3] = 0x46;
				a_uart[4] = 32;
				copy_bytes(&a_uart[5], p_tag->a_page[0], 4);
				copy_bytes(&a_uart[9], a_page1, 4);
				copy_bytes(&a_uart[13], a_page2, 4);
				copy_bytes(&a_uart[17], p_tag->a_page[3], 4);
			#if PCF7936_TYPE_NXP
				copy_bytes(&a_uart[21], a_page4_5_6_7[0], 4);
				copy_bytes(&a_uart[25], a_page4_5_6_7[1], 4);
				copy_bytes(&a_uart[29], a_page4_5_6_7[2], 4);
				copy_bytes(&a_uart[33], a_page4_5_6_7[3], 4);
			#else
				copy_bytes(&a_uart[21], a_page1, 4);
				copy_bytes(&a_uart[25], a_page1, 4);
				copy_bytes(&a_uart[29], a_page1, 4);
				copy_bytes(&a_uart[33], a_page1, 4);
			#endif
			//	a_uart[37] = calculate_sum(a_uart, 37);
			//	uart_send_str(a_uart, 38);

				lcd_print_hex(30, 40,200,24,24, p_tag->a_page[0], 4);

				i = 0;
				while (i < 3) {
					pcf7936_off_to_wait();
					if (pcf7936_auth_half(p_tag))
						i = 0;
					else
						i++;
					pcf7936_to_off();
				}
				//led_off();
				lcd_print_blank(1);
				state = s_exit;
				break;
				
			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}
}

void ts46_init_tag_v15(pcf7936_tag_t *p_tag)
{
	#if PCF7936_TYPE_NXP
	static const uint8_t a_page1[4] = {0x4D, 0x49, 0x4B, 0x52};
	static const uint8_t a_page2[4] = {0x96, 0xF8, 0x4F, 0x4E};
	static const uint8_t a_page3[4] = {0x06, 0xAA, 0x48, 0x54};
	
	static const uint8_t a_page4[4] = {0x46, 0x5F, 0x4F, 0x4B};
	static const uint8_t a_page5[4] = {0x55, 0x55, 0x55, 0x55};
	static const uint8_t a_page6[4] = {0xAA, 0xAA, 0xAA, 0xAA};
	static const uint8_t a_page7[4] = {0xFF, 0xFF, 0xFF, 0xFF};
	#else
	static const uint8_t a_page1[4] = {0xFF, 0xFF, 0xFF, 0xFF};
	static const uint8_t a_page2[4] = {0x96, 0xF8, 0xFF, 0xFF};
	static const uint8_t a_page3[4] = {0x0E, 0xAA, 0x48, 0x54};
	#endif
	//static const uint8_t a_page15[4] = {0x96, 0x7F, 0x00, 0x00};	//V1.5
	static const uint8_t a_page15[4] = {0x96, 0x0F, 0x00, 0x00};

	static uint8_t a_uart[40];
	uint8_t a_page0[4], a_rd[4];
	uint8_t i;
	uint16_t random;
	uint8_t (* pf_auth)(pcf7936_tag_t *);
		
	enum {
		s_check_test_mode, s_read_tag,
		s_init_all, s_init_p0, s_init_p1, s_init_p2, s_init_p3,
		s_init_p4, s_init_p5, s_init_p6, s_init_p7, s_init_p15, 
		s_init_ok, s_exit
	} state;

	pcf7936_clear_tag_flag(p_tag);
	state = s_check_test_mode;
	while (state != s_exit) {
		switch (state) {
			case s_check_test_mode:
				if (ts46_test_read(15, a_rd))
					state = s_init_all;
				else
					state = s_read_tag;
				break;

			case s_read_tag:
				p_tag->tmcf_dcs = e_modu_mc;
			#if PCF7936_TYPE_NXP
				p_tag->tmcf_enc = e_pcf7936_psw;
				copy_bytes(p_tag->a_page[1], a_page1, 4);
				pf_auth = pcf7936_auth_psw;
			#else
				p_tag->tmcf_enc = e_pcf7936_cipher;
				copy_bytes(p_tag->secret_key, &a_page2[2], 2);
				copy_bytes(&(p_tag->secret_key[2]), a_page1, 4);
				pf_auth = pcf7936_auth_cipher;
			#endif
				pcf7936_off_to_wait();
				if (!pf_auth(p_tag))
					state = s_exit;
				/* decide exit test mode */
				else if (!pcf7936_read_page(4, p_tag))
					state = s_exit;
				else if (p_tag->a_page[0][0] == p_tag->a_page[0][1])
					state = s_exit;
				else
					state = s_init_ok;
				pcf7936_to_off();
				break;

			case s_init_all:
				#if 0
				if (ts46_test_write(0, a_page_all, TRUE))
					state = s_init_p0;
				else if (ts46_test_write(0, a_page_all, TRUE))
					state = s_init_p0;
				else
					state = s_exit;
				#else
				state = s_init_p0;
				#endif
				break;
			case s_init_p0:
				random = rand();
				a_page0[0] = (uint8_t)(random>>8);
				a_page0[1] = a_page0[0] ^ 0xA5;
				a_page0[2] = (uint8_t)random;
				a_page0[3] = (a_page0[2] & 0x0F) | 0x10; // see pcf7936 datasheet EEPROM Content Upon Delivery
				if (ts46_test_write(0, a_page0, FALSE))
					state = s_init_p1;
				else if (ts46_test_write(0, a_page0, FALSE))
					state = s_init_p1;
				else
					state = s_exit;
				break;
			case s_init_p1:
				if (ts46_test_write(1, a_page1, FALSE))
					state = s_init_p2;
				else if (ts46_test_write(1, a_page1, FALSE))
					state = s_init_p2;
				else
					state = s_exit;
				break;
			case s_init_p2:
				if (ts46_test_write(2, a_page2, FALSE))
					state = s_init_p3;
				else if (ts46_test_write(2, a_page2, FALSE))
					state = s_init_p3;
				else
					state = s_exit;
				break;
			case s_init_p3:
				if (ts46_test_write(3, a_page3, FALSE))
					state = s_init_p4;
				else if (ts46_test_write(3, a_page3, FALSE))
					state = s_init_p4;
				else
					state = s_exit;
				break;
			case s_init_p4:
			#if PCF7936_TYPE_NXP
				if (ts46_test_write(4, a_page4, FALSE))
					state = s_init_p5;
				else if (ts46_test_write(4, a_page4, FALSE))
					state = s_init_p5;
				else
					state = s_exit;
			#else
				state = s_init_p15;
			#endif
				break;
			case s_init_p5:
			#if PCF7936_TYPE_NXP
				if (ts46_test_write(5, a_page5, FALSE))
					state = s_init_p6;
				else if (ts46_test_write(5, a_page5, FALSE))
					state = s_init_p6;
				else
					state = s_exit;
			#else
				state = s_init_p15;
			#endif
				break;
			case s_init_p6:
			#if PCF7936_TYPE_NXP
				if (ts46_test_write(6, a_page6, FALSE))
					state = s_init_p7;
				else if (ts46_test_write(6, a_page6, FALSE))
					state = s_init_p7;
				else
					state = s_exit;
			#else
				state = s_init_p15;
			#endif
				break;
			case s_init_p7:
			#if PCF7936_TYPE_NXP
				if (ts46_test_write(7, a_page7, FALSE))
					state = s_init_p15;
				else if (ts46_test_write(7, a_page7, FALSE))
					state = s_init_p15;
				else
					state = s_exit;
			#else
				state = s_init_p15;
			#endif
				break;
			case s_init_p15:
				if (ts46_test_write(15, a_page15, FALSE))
					state = s_check_test_mode;
				else if (ts46_test_write(15, a_page15, FALSE))
					state = s_check_test_mode;
				else
					state = s_exit;
				break;
				
			case s_init_ok:
	//			led_on();

				a_uart[0] = 0xA5;
				a_uart[1] = 0xFF;
				a_uart[2] = 35;
				a_uart[3] = 0x46;
				a_uart[4] = 32;
				copy_bytes(&a_uart[5], p_tag->a_page[0], 4);
				copy_bytes(&a_uart[9], a_page1, 4);
				copy_bytes(&a_uart[13], a_page2, 4);
				copy_bytes(&a_uart[17], p_tag->a_page[3], 4);
			#if PCF7936_TYPE_NXP
				copy_bytes(&a_uart[21], a_page4, 4);
				copy_bytes(&a_uart[25], a_page5, 4);
				copy_bytes(&a_uart[29], a_page6, 4);
				copy_bytes(&a_uart[33], a_page7, 4);
			#else
				copy_bytes(&a_uart[21], a_page1, 4);
				copy_bytes(&a_uart[25], a_page1, 4);
				copy_bytes(&a_uart[29], a_page1, 4);
				copy_bytes(&a_uart[33], a_page1, 4);
			#endif
			//	a_uart[37] = calculate_sum(a_uart, 37);
			//	uart_send_str(a_uart, 38);

				lcd_print_hex(30, 40,200,24,24, p_tag->a_page[0], 4);

				i = 0;
				while (i < 3) {
					pcf7936_off_to_wait();
					if (pcf7936_auth_half(p_tag))
						i = 0;
					else
						i++;
					pcf7936_to_off();
				}
			//	led_off();
				lcd_print_blank(1);
				state = s_exit;
				break;
				
			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}
}

uint8_t pcf7936_comm_handle(const uint8_t *p_ask, uint8_t *p_answer)
{
	uint8_t len_send, i, offset=0;
	uint8_t a_temp[6];
	uint8_t (* p_fun)(pcf7936_tag_t *)=NULL;

	/* password case */
	if ((p_ask[0] & 0xF0) == 0x10) {
		pcf7936_tag1.tmcf_enc = e_pcf7936_psw;
		p_fun = pcf7936_auth_psw;
		pcf7936_tag1.a_page[1][0] = p_ask[1];
		pcf7936_tag1.a_page[1][1] = p_ask[2];
		pcf7936_tag1.a_page[1][2] = p_ask[3];
		pcf7936_tag1.a_page[1][3] = p_ask[4];
		offset = 4;
	}
	/* cipher case */
	else if ((p_ask[0] & 0xF0) == 0x20) {
		pcf7936_tag1.tmcf_enc = e_pcf7936_cipher;
		p_fun = pcf7936_auth_cipher;
		pcf7936_tag1.secret_key[0] = p_ask[1];
		pcf7936_tag1.secret_key[1] = p_ask[2];
		pcf7936_tag1.secret_key[2] = p_ask[3];
		pcf7936_tag1.secret_key[3] = p_ask[4];
		pcf7936_tag1.secret_key[4] = p_ask[5];
		pcf7936_tag1.secret_key[5] = p_ask[6];
		offset = 6;
	}

	len_send = 1;
	p_answer[0] = 0x00;
	/* read page0 */
	if (p_ask[0] == 0x01) {
		if (pcf7936_decide_modu(&pcf7936_tag1)) {
			len_send = 6;
			p_answer[0] = p_ask[0];
			p_answer[1] = pcf7936_tag1.tmcf_dcs;
			copy_bytes(&p_answer[2], &(pcf7936_tag1.a_page[0][0]), 4);
		}
	}
	/* send current crypto */
	else if (p_ask[0] == 0x02) {
		len_send = 7;
		p_answer[0] = p_ask[0];
		copy_bytes(&p_answer[1], pcf7936_tag1.secret_key, 6);
	}
	/* send last copy crypto */
	else if (p_ask[0] == 0x03) {
//		eep_rd_data(EEP_TS46_SKRY, a_temp, 6);
		len_send = 7;
		p_answer[0] = p_ask[0];
		copy_bytes(&p_answer[1], a_temp, 6);
	}
	/*  */
	else if (p_ask[0] == 0x04) {
		update_wr_time(p_ask[1], p_ask[2], p_ask[3]);
		p_answer[0] = p_ask[0];
	}
	/* read page3 */
	else if ((p_ask[0] == 0x10) || (p_ask[0] == 0x20)) {
		if (pcf7936_decide_modu(&pcf7936_tag1)) {
			pcf7936_off_to_wait();
			if (p_fun(&pcf7936_tag1)) {
				len_send = 5;
				p_answer[0] = p_ask[0];
				copy_bytes(&p_answer[1], &(pcf7936_tag1.a_page[3][0]), 4);
			}
			pcf7936_to_off();
		}
	}
	/* read page 4to7 */
	else if ((p_ask[0] == 0x11) || (p_ask[0] == 0x21)) {
		if (!pcf7936_decide_modu(&pcf7936_tag1))
			;
		else if (!pcf7936_read_tag(&pcf7936_tag1, 4, 7))
			;
		else {
			len_send = 17;
			p_answer[0] = p_ask[0];
			copy_bytes(&p_answer[1], &(pcf7936_tag1.a_page[4][0]), 16);
		}
	}
	/* read page 1to2 */
	else if ((p_ask[0] == 0x12) || (p_ask[0] == 0x22)) {
		if (!pcf7936_decide_modu(&pcf7936_tag1))
			;
		else if (!pcf7936_read_tag(&pcf7936_tag1, 1, 2))
			;
		else {
			len_send = 9;
			p_answer[0] = p_ask[0];
			copy_bytes(&p_answer[1], &(pcf7936_tag1.a_page[1][0]), 8);
		}
	}
	/* write page 4 to 7 */
	else if ((p_ask[0] == 0x13) || (p_ask[0] == 0x23)) {
		if (pcf7936_decide_modu(&pcf7936_tag1)) {
			pcf7936_off_to_wait();
			if (p_fun(&pcf7936_tag1)) {
				for (i = 4; i < 8; i++) {
					delay_us(PCF7936_T_WAIT_BS);
					if (!pcf7936_write_page(i, &pcf7936_tag1, &p_ask[i*4 - 15 + offset]))
						break;
				}
				if (i == 8) {
					len_send = 1;
					p_answer[0] = p_ask[0];
				}
			}
			pcf7936_to_off();
		}
	}
	/* write page 0 */
	else if ((p_ask[0] == 0x14) || (p_ask[0] == 0x24)) {
		if (pcf7936_decide_modu(&pcf7936_tag1)) {
			pcf7936_off_to_wait();
			if (p_fun(&pcf7936_tag1)) {
				delay_us(PCF7936_T_WAIT_BS);
				if (pcf7936_write_page(0, &pcf7936_tag1, &p_ask[1 + offset])) {
					len_send = 1;
					p_answer[0] = p_ask[0];
				}
			}
			pcf7936_to_off();
		}
	}
	/* write page 1 */
	else if ((p_ask[0] == 0x15) || (p_ask[0] == 0x25)) {
		if (pcf7936_decide_modu(&pcf7936_tag1)) {
			pcf7936_off_to_wait();
			if (p_fun(&pcf7936_tag1)) {
				delay_us(PCF7936_T_WAIT_BS);
				if (pcf7936_write_page(1, &pcf7936_tag1, &p_ask[1 + offset])) {
					len_send = 1;
					p_answer[0] = p_ask[0];
				}
			}
			pcf7936_to_off();
		}
	}
	/* write page 2 */
	else if ((p_ask[0] == 0x16) || (p_ask[0] == 0x26)) {
		if (pcf7936_decide_modu(&pcf7936_tag1)) {
			pcf7936_off_to_wait();
			if (p_fun(&pcf7936_tag1)) {
				delay_us(PCF7936_T_WAIT_BS);
				if (pcf7936_write_page(2, &pcf7936_tag1, &p_ask[1 + offset])) {
					len_send = 1;
					p_answer[0] = p_ask[0];
				}
			}
			pcf7936_to_off();
		}
	}
	/* write page 3 */
	else if ((p_ask[0] == 0x17) || (p_ask[0] == 0x27)) {
		if (pcf7936_decide_modu(&pcf7936_tag1)) {
			pcf7936_off_to_wait();
			if (p_fun(&pcf7936_tag1)) {
				delay_us(PCF7936_T_WAIT_BS);
				if (pcf7936_write_page(3, &pcf7936_tag1, &p_ask[1 + offset])) {
					len_send = 1;
					p_answer[0] = p_ask[0];
				}
			}
			pcf7936_to_off();
		}
	}
	/* taisu new 06 test read */
	else if (p_ask[0] == 0x30) {
		if (ts06new_test_read(p_ask[1], &p_answer[1])) {
			len_send = 5;
			p_answer[0] = p_ask[0];
		}
	}
	/* taisu new 06 test write */
	else if (p_ask[0] == 0x31) {
		if (ts06new_test_write(p_ask[1], &p_ask[2])) {
			p_answer[0] = p_ask[0];
		}
	}
	
	return len_send;
}


/* some pcf7936 user data */
#if 0
********** pcf7936-password **********
CB 15 4D 13
4D 49 4B 52	// MIKR
20 28 4F 4E // --ON
06 AA 48 54
44 44 44 44
55 55 55 55
AA AA AA AA
FF FF FF FF
********** pcf7936-crypto **********
E3 9A 59 18
07 23 0A 0E
00 28 19 73
0F 51 03 28
44 44 44 44
55 55 55 55
20 28 AA AA
FF FF FF FF
********** 46-honda car **********
8E 06 79 88
07 23 0A 0E
00 00 19 73
0F 51 03 28
3C 22 2A 02
08 55 FF FF
03 03 03 0F
02 4C 4A 1B
********** 46-c3+ **********
B8 71 7B 69
10 7D 66 17
00 00 AE 5C
FE B2 48 57
55 0D 3B 2C
00 00 00 00
00 00 00 00
00 00 00 00
********** 46-c3+(WuYi) **********
6D B7 7B 1C
10 7D 66 17
00 00 AE 5C
FE B2 48 57
46 5F 4F 4B
55 55 55 55
AA AA AA AA
FF FF FF FF
(FastCrack:Random(7141A152),Sign(3B644774))
********** TOYOTA KEY **********
B5 A9 7B 1C
10 7D 66 17
00 00 AE 5C
FE B2 48 57
46 5F 4F 4B
55 55 55 55
AA AA AA AA
FF FF FF FF
********** NXP ORIGIN **********
B5 A9 7B 1C
4D 49 4B 52 // MIKR
00 00 4F 4E // --ON
06 AA 48 54
46 5F 4F 4B
55 55 55 55
AA AA AA AA
FF FF FF FF
********** JEEP **********
41 9A 75 6D
D8 49 BE 99
00 00 50 D2
00(-->0E) AA 48 54
46 5F 4F 4B
55 55 55 55
AA AA AA AA
FF FF FF FF
(FastCrack:Random(45A58363),Sign(C3D81EFD))
#endif


