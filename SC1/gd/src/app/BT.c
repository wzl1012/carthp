#include "headfile.h"

void BTR_T_task(void *pvParameters)
{
	BT_R_check();
}

void BT_R_check(void)
{
	uint32_t err=pdFALSE;
	uint8_t tmpdata=0;
	while(1){
					err=ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
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
			}
	}		
}
bool  BT_send(uint8_t* pdata,uint8_t len,uint8_t pktype,uint8_t rdcmd)
{
	uint8_t* tmpdata=NULL/*tmppri=NULL*/;
	uint8_t  s_sz=0/*tmpsz=0*/;
	if(pktype==0xd2){
		s_sz=len+9;		
	}else if(pktype==0xd3){
			s_sz=len+8;
	}
	tmpdata=malloc(s_sz);
	tmpdata[0]=0xfd;
  tmpdata[1]=0xdf;
  tmpdata[2]=s_sz;
	tmpdata[5]=pktype;
		if(pktype==0xd2){	
		tmpdata[6]=rdcmd;
		for(uint8_t i=0;i<len;i++)
    tmpdata[7+i]=pdata[i];
	}else if(pktype==0xd3){
		for(uint8_t i=0;i<len;i++)
    tmpdata[6+i]=pdata[i];
	}
	tmpdata[3]=(checksum(&tmpdata[5],s_sz-7)&0xFF00)>>8;
  tmpdata[4]=(checksum(&tmpdata[5],s_sz-7)&0xFF);
  tmpdata[s_sz-2]=0xaa;
  tmpdata[s_sz-1]=0x55;	
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















