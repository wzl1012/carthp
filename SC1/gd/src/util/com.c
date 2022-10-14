#include "debprint.h"
#include "cmm.h"
void __no_operation(void)
{
	;
}
void void_printf(void*f,...)
{
}
uint16_t checksum(uint8_t *u2rev,uint8_t datasz)
{
	uint16_t checksum=0;
  for(uint8_t i=0;i<datasz; i++){
		  checksum+=u2rev[i];
	}
  return checksum; 	
}
uint8_t v_to_per(float v_value)
{
	uint16_t tmp=(uint16_t)(v_value*10000);
	if(tmp>=MAX_LV){
     return 100;
	}else if(tmp<MAX_LV&&tmp>=MIN_LV){
		   if(((tmp-MIN_LV)%(STEPV))==0)
		   return((tmp-MIN_LV)/(STEPV));
			 else
			 return((tmp-MIN_LV)/(STEPV)+1);	 
	}else if(tmp<MIN_LV){
		  return 0;		
	}	
	return FALSE;
}
/*
static bool asc_to_dec(char inpasc,uint8_t*dec)
{
	if((inpasc>='0')&&(inpasc<='9')){
		 *dec=inpasc-'0';
		 return TRUE;
	}else{
		return FALSE;
	}
}*/

