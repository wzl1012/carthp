#include "headfile.h"

#define L_BDRNUM 8
//#define  BT_LKSTA  gpio_input_bit_get(GPIOC,GPIO_PIN_2)

static bool BT_u2sndr_cmd(char*cmd,char* rspd)
{	
	uint8_t cmdlen=0,rev_datalen=0;
	uint16_t  timeout=50000;
	char  uartrevbuf[30]={0};
	 cmdlen=strlen(cmd)/*+strlen(data)*/;
	  /*if(strlen(data)==0){
		 rev_datalen=cmdlen+2;
	 }else{
		 rev_datalen=cmdlen;
	 }*/
	//if(usart_flag_get(USART1,USART_FLAG_RBNE)){
		 usart_data_receive(USART1);
	//	usart_flag_clear(USART1,USART_FLAG_RBNE);
	//}
	rev_datalen=strlen(rspd);
	fputc_u2(cmd,cmdlen);
	for(uint8_t i=0;i<rev_datalen;i++){
			 while(usart_flag_get(USART1,USART_FLAG_RBNE)==RESET&&timeout){
				    delay_us(10);
				 	  timeout--;
				}
			if(timeout==0)
				break;
      else{  
				      // usart_flag_clear(USART1,USART_FLAG_RBNE);
					     uartrevbuf[i]=(uint8_t)usart_data_receive(USART1);
				       timeout=50000;				        
					    }
	}	
//debug_printf("\r\nrevch=%s",uartrevbuf);
     if(strcmp(uartrevbuf,rspd/*"OK+S_BAUD=7"*/)==0){		 
					  return TRUE;
	    }/*else if(strcmp(uartrevbuf,"OK+S_NAME=syz01")==0){
				    return TRUE;
			}*/else{
				  return FALSE;
			}
}
static char sendbuf[30];
static char ackbuf[30];
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

static bool BT_ubdrate(void)
{
	sprintf((char *)sendbuf, "AT+BAUD?\r\n");
	sprintf((char *)ackbuf, "OK+G_BAUD=7\r\n");
		if(BT_u2sndr_cmd(sendbuf,ackbuf)){
		 return TRUE;
	}else{
		 return FALSE;
	}
}
uint8_t BT_UARTIF_init(void)
{
	BT_MOD_reset();
	if(!BT_ubdrate()){
		/*uint8_t dec=0;
		if(!asc_to_dec(ackbuf[0],&dec)){
			return FALSE;
		}*/
	uint8_t  bdratenum=0;
	uint32_t bdratelst[L_BDRNUM]={
	  2400,4800,9600,19200,38400,57600,115200,128000
};
	while(bdratenum<L_BDRNUM){
		      usart_disable(USART1);
				 	usart_baudrate_set(USART1,bdratelst[bdratenum]);
          usart_enable(USART1);		
		      sprintf((char *)sendbuf, "AT+BAUD=7\r\n");
		      sprintf((char *)ackbuf, "OK+S_BAUD=7\r\n");
          if(BT_u2sndr_cmd(sendbuf,ackbuf)){
						if(bdratelst[bdratenum]!=115200)
						 usart_baudrate_set(USART1,115200);	
						 BT_MOD_reset();					
					   return TRUE;
				  }else{
					++bdratenum;
				 }
		  }
     return FALSE;	
	 }
	return TRUE;
}	
			 
void BT_MOD_reset(void)
{
	 gpio_bit_reset(GPIOC,GPIO_PIN_3);
	 delay_us(100000);
	 gpio_bit_set(GPIOC,GPIO_PIN_3);
	 delay_us(2000000);
}
static bool BT_devname(void)
{
	sprintf((char *)sendbuf, "AT+NAME?\r\n");
	sprintf((char *)ackbuf, "OK+G_NAME=K3-Genie\r\n"/*,temp[0],temp[1],temp[2]*/);
		if(BT_u2sndr_cmd(sendbuf,ackbuf)){
		 return TRUE;
	}else{
		 return FALSE;
	}
}
/*
static bool BT_chkbtv(void)
{
	sprintf((char *)sendbuf, "AT+VERS?\r\n");
	sprintf((char *)ackbuf, "OK+G_NAME=syz-vvvvvvvvv\r\n");
		if(BT_u2sndr_cmd(sendbuf,ackbuf)){
		 return TRUE;
	}else{
		 return FALSE;
	}
}*/
bool set_BTname(void)
{
	//BT_chkbtv( );
	if(!BT_devname()){
	sprintf((char *)sendbuf, "AT+NAME=K3-Genie\r\n"/*,temp[0],temp[1],temp[2]*/);
	sprintf((char *)ackbuf, "OK+S_NAME=K3-Genie\r\n"/*,temp[0],temp[1],temp[2]*/);
	if(BT_u2sndr_cmd(sendbuf,ackbuf)){
		BT_MOD_reset( );
		 return TRUE;
	}else{
		 return FALSE;
	}
 }else{
	 return TRUE;
 }
}
bool BT_disc(void)
{
	sprintf((char *)sendbuf, "AT+DISC\r\n");
	sprintf((char *)ackbuf, "OK+DISC\r\n");
		if(BT_u2sndr_cmd(sendbuf,ackbuf)){
		 return TRUE;
	}else{
		 return FALSE;
	}
}

void BT_R_check(void)
{
	uint32_t err=pdFALSE;
	uint8_t tmpdata=0;
	//ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
	while(1){
		// if(BT_LKSTA){
		//debug_printf("\r\nurs=%04x",USART_RX_STA);
		    //  if(i==35)
				//	debug_printf("\r\nno35");
					err=ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
		    // debug_printf("\r\nerr=%d,rst=%x,i=%d",err,USART_RX_STA,i++);
         if(err==1){						
					nvic_irq_disable(USART1_IRQn); 
			   if((USART_RX_STA&0x2000)==0x2000||BT_recvdatadec(USART_RX_BUF)==FALSE){
					 tmpdata=0xe3;
					 BT_send(&tmpdata,1,0xd3,0);
				 }
				 memset(USART_RX_BUF,0,sizeof(USART_RX_BUF));
				 USART_RX_STA=0x0;
				 nvic_irq_enable(USART1_IRQn,7,2);
			  //}
				  debug_printf("\r\ntask2");
			}
		//portBASE_TYPE uxHighWaterMark;
    //uxHighWaterMark=uxTaskGetStackHighWaterMark(BTR_T_Task_Handler );
		//vProtFree(BTR_T_Task_Handler);
    //debug_printf("\r\ntasksz=%d",(uint16_t)uxHighWaterMark);
		//debug_printf("\r\ntask2end");		
	// }
	//vTaskDelay(10);	
	}		
}
bool  BT_send(uint8_t* pdata,uint8_t len,uint8_t pktype,uint8_t rdcmd)
{
	uint8_t* tmpdata=NULL/*tmppri=NULL*/;
	uint8_t  s_sz=0/*tmpsz=0*/;
	if(pktype==0xd2){
		s_sz=len+9;		
		//tmpdata[7]=pdata[len-1];
	}else if(pktype==0xd3){
			s_sz=len+8;
	}
	tmpdata=malloc(s_sz);
	tmpdata[0]=0xfd;
  tmpdata[1]=0xdf;
  tmpdata[2]=s_sz;
	tmpdata[5]=pktype;
		if(pktype==0xd2){	
		//tmpdata[7]=pdata[len-1];
		tmpdata[6]=rdcmd;
		for(uint8_t i=0;i<len;i++)
    tmpdata[7+i]=pdata[i];
	}else if(pktype==0xd3){
		for(uint8_t i=0;i<len;i++)
    tmpdata[6+i]=pdata[i];
	}
	//tmpdata[6]=rdcmd;
 	//tmpdata[6]=pdata[len-1];
	tmpdata[3]=(checksum(&tmpdata[5],s_sz-7)&0xFF00)>>8;
  tmpdata[4]=(checksum(&tmpdata[5],s_sz-7)&0xFF);
  tmpdata[s_sz-2]=0xaa;
  tmpdata[s_sz-1]=0x55;	
	//tmpsz=s_sz;
	//debug_printf("\r\nbtsd=");
	//tmppri=tmpdata;
	//while(tmpsz--)
	//debug_printf("%02x",*(tmppri++));
	u2_sendhex(tmpdata,s_sz);
	free(tmpdata);
	return TRUE;
}
static bool em4170_btsend(em4170_tag_t* em4170_tag,uint8_t rdcmd)
{
	if (em4170_tag->mode == e_no_em4170)
		return FALSE;
	switch(rdcmd){
	  case(0xa1):{
			uint8_t s_data[11]={0};
			s_data[0]=0x48;
			s_data[1]=0x00;
			s_data[2]=em4170_tag->a_id[0];
			s_data[3]=em4170_tag->a_id[1];
	    s_data[4]=em4170_tag->a_id[2];
	    s_data[5]=em4170_tag->a_id[3];
			 s_data[6]=0x00;
			if((em4170_tag->a_um1[3]&0xc0)==0xc0){
			  s_data[7]=0x02;
				s_data[8]=0x00;
			}else{
				s_data[7]=0x01;
				s_data[8]=0x01;
			}
			//s_data[2]=0x01;
				s_data[9]=0x00;
				s_data[10]=0x00;
		 BT_send(s_data,11,0xd2,rdcmd);
	  }
		return TRUE;
	  //break;
	 	case(0xa2):{
		 	uint8_t s_data[15][3]={{0x00,0x48,0x00},{0x10,em4170_tag->a_id[0],em4170_tag->a_id[1]},{0x11,em4170_tag->a_id[2],em4170_tag->a_id[3]},
			                       {0x12,em4170_tag->a_um1[0],em4170_tag->a_um1[1]},{0x13,em4170_tag->a_um1[2],em4170_tag->a_um1[3]},
														 {0x14,em4170_tag->a_um2[0],em4170_tag->a_um2[1]},{0x15,em4170_tag->a_um2[2],em4170_tag->a_um2[3]},
														 {0x16,em4170_tag->a_um2[4],em4170_tag->a_um2[5]},{0x17,em4170_tag->a_um2[6],em4170_tag->a_um2[7]},
														 {0x18,em4170_tag->key[0],em4170_tag->key[1]},{0x19,em4170_tag->key[2],em4170_tag->key[3]},
														 {0x1a,em4170_tag->key[4],em4170_tag->key[5]},{0x1b,em4170_tag->key[6],em4170_tag->key[7]},
														 {0x1c,em4170_tag->key[8],em4170_tag->key[9]},{0x1d,em4170_tag->key[10],em4170_tag->key[11]}
			};
      //s_data[14][0]=0x02;			
		 BT_send(s_data[0],45,0xd2,rdcmd);
	  }
		return TRUE;
	 // break;
		default:
		  return FALSE;	
	}	
}
static bool tms37145_btsend(tms37145_tag_t* tms37145_tag,uint8_t rdcmd)
{
		if (tms37145_tag->mode == e_tms37145_not)
		return FALSE;
		switch(rdcmd){
	  case(0xa1):{
			uint8_t s_data[13]={0};
				if (( tms37145_tag->mode == e_b9wk) || (tms37145_tag->mode == e_tms37145_b9wk)) {
					   		if (tms37145_tag->crack_flag) {
			                s_data[0]=tms37145_tag->page4[1];
			                s_data[1]=tms37145_tag->page4[0];
		            }
		           else if (tms37145_tag->mode == e_tms37145_b9wk&&tms37145_tag->page3[3] != 0x01) {
			             s_data[0]=0x4D;
                   s_data[1]=0x40;
		           }
		          else if (tms37145_tag->read_len == 8){
			             s_data[0]=0x4D;
                   s_data[1]=0x64;
		          }
		          else if (tms37145_tag->page3[3] == 0x01){
			 			       s_data[0]=0x4D;
                   s_data[1]=0x63;
		          }
		          else if (tms37145_tag->read_len == 10){
						       s_data[0]=0x4D;
                   s_data[1]=0x00;
		          }	
			}else if ((tms37145_tag->mode == e_tms37145_pwm) || (tms37145_tag->mode == e_tms37145_ppm)) {
					  s_data[0]=0x4D;
            s_data[1]=0x80;
			}	else
		     return FALSE;
			s_data[2]=tms37145_tag->page1[0];
			s_data[3]=tms37145_tag->page2[0];
	    s_data[4]=tms37145_tag->page3[0];
	    s_data[5]=tms37145_tag->page3[1];
      s_data[6]=tms37145_tag->page3[2];	
      s_data[7]=tms37145_tag->page3[4];	
			s_data[8]=0x00;
			 if(tms37145_tag->crack_flag){
			  s_data[9]=0x01;
				s_data[10]=0x01;
			}else{
				s_data[9]=0x02;
				s_data[10]=0x00;
			}
			     s_data[11]=0x00;
				   s_data[12]=0x01;
		 BT_send(s_data,13,0xd2,rdcmd);
	  }
		return TRUE;
	 // break;
	 	case(0xa2):{
		 	uint8_t s_data[4][3]={{0x00,0x4D,0x00},{0x10,tms37145_tag->page1[0],tms37145_tag->page2[0]},{0x11,tms37145_tag->page3[0],tms37145_tag->page3[1]},
			                       {0x12,tms37145_tag->page3[2],tms37145_tag->page3[4]}
			};
     // s_data[3][0]=0x02;			
		 BT_send(s_data[0],12,0xd2,rdcmd);
	  }
		return TRUE;
	 // break;
		default:
			return FALSE;	
	}	
}
static bool pcf7936_btsend(pcf7936_tag_t* pcf7936_tag,uint8_t rdcmd)
{
		if ( pcf7936_tag->mode == e_pcf7936_not)
		return FALSE;
		switch(rdcmd){
	  case(0xa1):{
		 			uint8_t s_data[17]={0};
			s_data[0]=0x46;
			s_data[1]=0x00;
			s_data[2]=pcf7936_tag->a_page[0][0];
			s_data[3]=pcf7936_tag->a_page[0][1];
	    s_data[4]=pcf7936_tag->a_page[0][2];
	    s_data[5]=pcf7936_tag->a_page[0][3];
			s_data[6]=pcf7936_tag->a_page[2][2];
s_data[7]=pcf7936_tag->a_page[2][3];
s_data[8]=pcf7936_tag->a_page[1][0];
s_data[9]=pcf7936_tag->a_page[1][1];
s_data[10]=pcf7936_tag->a_page[1][2];
s_data[11]=pcf7936_tag->a_page[1][3];					
      s_data[12]=0x01;					
			 if((pcf7936_tag->a_page[3][0]&0xF0)==0x00){
			  s_data[13]=0x01;
				s_data[14]=0x01;
			}else{
				s_data[13]=0x02;
				s_data[14]=0x00;
			}
			 if((pcf7936_tag->a_page[3][0]&0x08)==0x00){
				   s_data[15]=0x01;
			 }else{
				   s_data[15]=0x02;
			 }
			 if((pcf7936_tag->a_page[3][0]&0x01)==0x00){
				   s_data[16]=0x01;
			 }else{
				   s_data[16]=0x02;
			 }
			//s_data[2]=0x01;
		 BT_send(s_data,17,0xd2,rdcmd);
				return TRUE;
	  }
	 // break;
	 	case(0xa2):{
		   		 	uint8_t s_data[17][3]={{0x00,0x46,0x00},{0x10,pcf7936_tag->a_page[0][0],pcf7936_tag->a_page[0][1]},{0x11,pcf7936_tag->a_page[0][2],pcf7936_tag->a_page[0][3]},
			                       {0x12,pcf7936_tag->a_page[1][0],pcf7936_tag->a_page[1][1]},{0x13,pcf7936_tag->a_page[1][2],pcf7936_tag->a_page[1][3]},
														 {0x14,pcf7936_tag->a_page[2][0],pcf7936_tag->a_page[2][1]},{0x15,pcf7936_tag->a_page[2][2],pcf7936_tag->a_page[2][3]},
														 {0x16,pcf7936_tag->a_page[3][0],pcf7936_tag->a_page[3][1]},{0x17,pcf7936_tag->a_page[3][2],pcf7936_tag->a_page[3][3]},
														 {0x18,pcf7936_tag->a_page[4][0],pcf7936_tag->a_page[4][1]},{0x19,pcf7936_tag->a_page[4][2],pcf7936_tag->a_page[4][3]},
														 {0x1a,pcf7936_tag->a_page[5][0],pcf7936_tag->a_page[5][1]},{0x1b,pcf7936_tag->a_page[5][2],pcf7936_tag->a_page[5][3]},
														 {0x1c,pcf7936_tag->a_page[6][0],pcf7936_tag->a_page[6][1]},{0x1d,pcf7936_tag->a_page[6][2],pcf7936_tag->a_page[6][3]},
														 {0x1e,pcf7936_tag->a_page[7][0],pcf7936_tag->a_page[7][1]},{0x1f,pcf7936_tag->a_page[7][2],pcf7936_tag->a_page[7][3]}
			      };
		 // s_data[16][0]=0x02;			
		  BT_send(s_data[0],51,0xd2,rdcmd);
		  return TRUE;
					}
	   // break;
		  default:
		  return FALSE;		
	  }
}
static bool state_btsend(state_eum state)
{
	uint8_t tmpstate=(uint8_t)state;
	if(BT_send(&tmpstate,1,0xd3,NOCMD))
		  return TRUE;
  else
      return FALSE;		
}
void  BT_rdcmd_exe(uint8_t rdcmd)
{
	#define TRY_TIME	(2)	// in order to read stable
  uint8_t i,ttf,d7991_find_ok = FALSE,d3705_find_OK = FALSE;
	uint8_t scantimes=2;
	ICD_5V_ON;
	pcf7991_init();	 
	tms3705_init();	
while(scantimes--){
		for (i = 0; (i<TRY_TIME); i++){ 
	    ttf = pcf7991_turn_dout(50, PCF7991_PAGE0_NORMAL, 20, 10);
		}     
	/* ASK tag talk first */
 if (ttf) {
		/* em4170(48) */
			if(!DEV_SELPIN_level){
		    for (i = 0; (i<TRY_TIME)&&(!d7991_find_ok); i++) {
			     em4170_find_tag(&em4170_tag1);
			     d7991_find_ok = em4170_btsend(&em4170_tag1,rdcmd);
		    }
		   if(d7991_find_ok){break;}
	   }
				/* new(48) */
		if(!DEV_SELPIN_level){
		   for (i = 0; (i<TRY_TIME)&&(!d7991_find_ok); i++) {
			 new48_find_tag(&em4170_tag1);
			 d7991_find_ok = em4170_btsend(&em4170_tag1,rdcmd);
		  }
			if(d7991_find_ok){break;}
    }
 }
 else {
			if(DEV_SELPIN_level){
  	   for (i = 0; (i<TRY_TIME)&&(!d3705_find_OK); i++) {
			   tms37145_find_tag(&tms37145_tag1);
			   d3705_find_OK = tms37145_btsend(&tms37145_tag1,rdcmd);
		    }
			  if(d3705_find_OK){break;}
	   }
		if(!DEV_SELPIN_level){
        ts06_pcf7936typeinit_tag(&pcf7936_tag1/*,pcf7936_yes,nocrty*/);
		    for (i = 0; (i<TRY_TIME)&&(!d7991_find_ok); i++) {
			//pcf7936_clear_tag_flag(&pcf7936_tag1);
			// pcf7991_confg();
			 pcf7936_find_tag(&pcf7936_tag1);
			 d7991_find_ok = pcf7936_btsend(&pcf7936_tag1,rdcmd);
		 }
		   if(d7991_find_ok){break;}
	 }
	/* if(!DEV_SELPIN_level){
			  pcf7953_init_tag(&pcf7953_tag);
		    for (i = 0; (i<TRY_TIME)&&(!d7991_find_ok); i++) {
		        pcf7953_find_tag(&pcf7953_tag);
          // d7991_find_ok = pcf7953_keypad_interaction(&pcf7953_tag);
         }
		    if(d7991_find_ok){break;}
		}*/
 }
	  if(DEV_SELPIN_level)
			   DEV_SEL_7991;
	  else
			  DEV_SEL_3705;
    delay_us(1000);		
}
	   if (!d7991_find_ok&&!d3705_find_OK)
		    state_btsend(rdfail);
		  ICD_5V_OFF;
}















