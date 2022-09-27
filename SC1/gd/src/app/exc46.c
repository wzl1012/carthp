#include "exc46.h"

void exc46_reset(exc46_tag_t *p_tag)
{
	pcf7991_confg();
	
	
}
static bool exc46_id(exc46_tag_t *p_tag)
{
		uint8_t a_read[5]={0},cmd;
	  cmd = EXCIDCMD;
	uint8_t result = FALSE;
	pcf7991_write_tag_n(0x07);
	delay_us(400);
	pcf7936_wr_msbits(&cmd,8, TRUE);
	pcf7991_tagrd_fsetproc();
	pcf7991_read_tag();
#if SYSTEM_SUPPORT_OS	
 taskENTER_CRITICAL();  
#endif
	if (pcf7991_decode_miss_low()) {
		if (p_tag->tmcf_dcs == e_modu_mc)
			result = pcf7991_decode_mc(32, a_read, 36, e_dir_msbit);
		else
			result = pcf7991_decode_bp(32, a_read, 36, e_dir_msbit);
	}
#if SYSTEM_SUPPORT_OS	
 taskEXIT_CRITICAL();
#endif	
	/* check read correct */
	if (!result)
		   ;
		//delay_us(20);
	/* check header '11111' */
	else if ((a_read[0]&0xF8) == 0xf0/*0xF8*/) {
		copy_msbits(p_tag->a_page[0], 0, a_read, 4, 32);
		//if(*((uint32_t*)(p_tag->a_page[0]))==EXCGIDC){
			    p_tag->exc46state=e_idcfm;
		//}
	}
	else
		result = FALSE;
  pcf7991_exit_tag_mode( );
	return result;	
}
static bool exc46_decide_modu(exc46_tag_t *p_tag)
{
	uint8_t result;
	result=FALSE;
	p_tag->tmcf_dcs = e_modu_mc;
	pcf7991_confg();
	 result =exc46_id(p_tag);
	pcf7936_to_off();
	if (!result) {
		p_tag->tmcf_dcs = e_modu_bp;
		//pcf7936_off_to_wait();
		pcf7991_confg();
		result = exc46_id(p_tag);
		pcf7936_to_off();
	}
	return result;
}
static bool exc46_auth(exc46_tag_t *p_tag)
{
	uint8_t a_read[5];
	uint8_t result = FALSE;
  // pcf7936_auth_half(p_tag);
	//delay_us(400);
	//pcf7991_exit_tag_mode( );
	pcf7991_confg();
	if (exc46_decide_modu(p_tag)){
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


