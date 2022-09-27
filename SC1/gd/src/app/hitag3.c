#include "headfile.h"
pcf7953_tag_t pcf7953_tag;
uint64_t state_7953=0;
void pcf7953_init_tag(pcf7953_tag_t *p_tag)
{
	p_tag->now_state = e_power_off;
	p_tag->mode = e_pcf7953_not;	
		  p_tag->random[0] = 0x66;
	    p_tag->random[1] = 0x97;
	    p_tag->random[2] = 0x50;
	    p_tag->random[3] = 0xEB;
		  p_tag->random[4] = 0xE1;
		  p_tag->random[5] = 0x93;
		  p_tag->random[6] = 0x46;
		  p_tag->random[7] = 0x00;
	p_tag->tmcf_enc=e_pcf7936_cipher;
	   /* p_tag->secret_key[0] = 0x52;
		  p_tag->secret_key[1] = 0x4b;
		  p_tag->secret_key[2] = 0x49;
		  p_tag->secret_key[3] = 0x4d;
		  p_tag->secret_key[4] = 0x4E;
		  p_tag->secret_key[5] = 0x4F;	*/
	    p_tag->secret_key[0] = 0x11;
		  p_tag->secret_key[1] = 0x11;
		  p_tag->secret_key[2] = 0x22;
		  p_tag->secret_key[3] = 0x22;
		  p_tag->secret_key[4] = 0x33;
		  p_tag->secret_key[5] = 0x33;
			p_tag->secret_key[6] = 0x44;
		  p_tag->secret_key[7] = 0x44;
		  p_tag->secret_key[8] = 0x55;
		  p_tag->secret_key[9] = 0x55;
			p_tag->secret_key[10] = 0x66;
		  p_tag->secret_key[11] = 0x66;
}
static uint8_t pcf7953_auth_half(pcf7953_tag_t *p_tag)
{
  uint8_t a_read[5]={0},cmd;
	cmd = 0xE0;
	uint8_t result = FALSE;
	//pcf7991_write_tag_n(0);
	pcf7991_write_tag_n(0x07);
	delay_us(400);
	pcf7936_wr_msbits(&cmd, 5, TRUE);
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
	else if ((a_read[0]&0xF0) == 0xf0/*0xF8*/) {
		p_tag->now_state = e_auth_half;
		copy_msbits(p_tag->a_page[0], 0, a_read, 4, 32);
    // copy_bytes(p_tag->a_page[0], a_read, 4);
	}
	else
		result = FALSE;
  pcf7991_exit_tag_mode( );
	return result;
	
}
uint8_t pcf7953_decide_modu(pcf7953_tag_t *p_tag)
{
		uint8_t result;
	result=FALSE;
	p_tag->tmcf_dcs = e_modu_mc;
	pcf7991_confg();
	 result =pcf7953_auth_half(p_tag);
	pcf7936_to_off();
	if (!result) {
		p_tag->tmcf_dcs = e_modu_bp;
		//pcf7936_off_to_wait();
		pcf7991_confg();
		result = pcf7953_auth_half(p_tag);
		pcf7936_to_off();
	}
	return result;
}
static uint8_t pcf7953_read_page(uint8_t page,pcf7953_tag_t *p_tag)
 {
    uint8_t tmp[2],a_read[5];
		uint8_t result = FALSE;
     uint16_t cmd = 0xC000;
	 	 if (page > 7)
		  return FALSE;
     cmd |= (page & 0x7) << 11 | (((~page) & 0x7) << 6);
     delay_us(PCF7936_T_WAIT_BS);
		 tmp[0]=(cmd&0xFF00)>>8;
		 tmp[1]=cmd&0xFF;
		 //cmd=(cmd&0xFFC)>>6;
		 if (p_tag->tmcf_enc == e_pcf7953_cipher)
			 hitag2_cipher_transcrypt(&state_7953,tmp, 2,0);
		    //hitag2_oneway2(cmd, 10);
	    pcf7991_write_tag_n(0x07);
	//pcf7991_write_tag_n(0);
	    delay_us(400);
	    pcf7936_wr_msbits(&tmp[0], 10, TRUE);
		 	pcf7991_tagrd_fsetproc();
#if SYSTEM_SUPPORT_OS	
 taskENTER_CRITICAL();  
#endif
	    pcf7991_read_tag(); 							// switching the device into transparent mode (READ_TAG mode)
		 if (pcf7991_decode_miss_low()) {
		if (p_tag->tmcf_dcs == e_modu_mc)
			result = pcf7991_decode_mc(32, a_read, 36/*37*/, e_dir_msbit);
		else
			result = pcf7991_decode_bp(32, a_read, 36/*37*/, e_dir_msbit);
	}
#if SYSTEM_SUPPORT_OS	
 taskEXIT_CRITICAL();
#endif
	if (result && ((a_read[0]&0xF0) ==0xf0/* 0xF8*/)) {
		copy_msbits(p_tag->a_page[page], 0, a_read,4/* 5*/, 32);
		//copy_bytes(p_tag->a_page[page], a_read, 4);
		if (p_tag->tmcf_enc == e_pcf7936_cipher)
			hitag2_oneway2(p_tag->a_page[page], 32);
	}
	else
		result = FALSE;

	return result;
}
 uint8_t pcf7953_auth_cipher(pcf7953_tag_t *p_tag)
{
	uint8_t result = FALSE;
	uint8_t a_write[4], a_read[5];
	pcf7991_confg();
	if (pcf7953_auth_half(p_tag)) {
		hitag2_cipher_reset_7953(p_tag);
		//}else if(p_tag->hitagmode==hitag3){
			//hitag2_2_set_sk(p_tag->secret_key ,96);
			//hitag3_3_set_random(p_tag->random);
	//	}
		 hitag2_cipher_authenticate(&state_7953, a_write);
		//hitag2_oneway2(a_write, 32);
				//if(p_tag->ictype==other_typeic){
					//a_write[0]=0xF9;
					//a_write[1]=0x41;
				//	hitag2_oneway2(a_write,16);
				//}else{
			   //hitag2_oneway2(a_write,32);
				//}
			//for(uint8_t i=0;i<4;i++)
			// debug_printf("\r\nencya_write[%d]=0x%02x\r\n",i,a_write[i]);
		   delay_us(PCF7936_T_WAIT_BS);				
		//pcf7991_write_tag_n(0x07);
		//pcf7991_write_tag_n(0);
	//	if(p_tag->ictype==other_typeic){
		    pcf7991_write_tag();
	//	}else{
			//pcf7991_write_tag_n(0x07);
	//	}
		//if(p_tag->hitagmode==hitag2){
		//pcf7936_wr_msbits(p_tag->random, 32, FALSE);	//send 32 bits random
		//pcf7936_wr_msbits(a_write, 32, TRUE);			//send 32 bits skey
		//}else if(p_tag->hitagmode==hitag3){
		  delay_us(400);
		  pcf7936_wr_msbits(p_tag->random, 64, TRUE);	//send 32 bits random	
		//}
		
		//pcf7991_set_page(0x00, g_pcf7991_p0_pcf7936);
		pcf7991_tagrd_fsetproc();
		//if(p_tag->ictype==other_typeic){
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
		//}
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
		if (result && ((a_read[0]&0xF0) == 0xf0/*0xF8*/)) {
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
static uint8_t pcf7953_read_tag(pcf7953_tag_t *p_tag, uint8_t start_p, uint8_t end_p)
{
	uint8_t result;
	uint8_t trys = 0;
	//uint8_t (* p_fun)(pcf7953_tag_t *);

	//if (p_tag->tmcf_enc == e_pcf7936_cipher)
		//p_fun = pcf7953_auth_cipher;
	//else
		//p_fun = pcf7953_auth_psw;
	
	result = FALSE;
	for (trys = 0; (trys<3) && (!result); trys++) {
		//pcf7936_off_to_wait();
			uint8_t i;
			for (i = start_p; i <= end_p; i++) {
				if (!pcf7953_read_page(i, p_tag))
					break;
			}
			if (i > end_p)
				result = TRUE;
			else
				result = FALSE;
		pcf7936_to_off();
	}

	return result;
} 
uint8_t pcf7953_find_tag(pcf7953_tag_t *p_tag)
{
	if (!pcf7953_decide_modu(p_tag))
		return FALSE;
	p_tag->mode = e_pcf7953_yes;
	if(!pcf7953_auth_cipher(p_tag))
		return FALSE;
 if (pcf7953_read_tag(p_tag, 0, 7)) {
			p_tag->now_state = e_read_all;
			p_tag->mode = e_pcf7953_cracked;
			return TRUE;
	    }	
 return TRUE;
}
