#include "headfile.h"
uint8_t pswlckflg=0;
uint8_t  cmdflg=0;
 enum {
		chekhd,
	  brcv,
	}krvstate=chekhd;
uint16_t checksum(uint8_t *u2rev,uint8_t datasz)
{
	uint16_t checksum=0;
  for(uint8_t i=0;i<datasz; i++){
		  checksum+=u2rev[i];
	}
  return checksum; 	
}

static float v_check(void)
{
	   /* ADC software trigger enable */
     adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
     ///* delay a time in milliseconds */
	    while(dma_flag_get(DMA0, DMA_CH0,DMA_FLAG_FTF)==RESET);
     //delay_us(1000*1000);
	   //adc_flag_clear(ADC0,ADC_FLAG_EOC);
     	dma_flag_clear(DMA0, DMA_CH0,DMA_FLAG_FTF);
	     //debug_printf("\r\n TCN2=%0x\r\n",TCNT2_V);
	   //adc_timer1trig_confg( );
	 //debug_printf("\r\n TCN2=%0x\r\n",TCNT2_V);
	   delay_us(1000);
	   if(adc_value[0]){
			 //adc_timer1trig_confg( );
			float tmpv=0;
      tmpv=(adc_value[0] * 3.3 / 4096);
			 //adc_value[0]=0;
			 return tmpv;
	 }
		 return FALSE;
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
uint8_t BATV_swtrigcheck(void )
{
	float tmpv=0;
		timer_disable(TIMER1);
		adc_disable(ADC0);
		//TCNT2=TIM1_PERID;
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);
		adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);
		adc_flag_clear(ADC0,ADC_FLAG_EOC);
	  //  delay_us(1000);
    /* ADC calibration and reset calibration */
   // adc_calibration_enable(ADC0);
	  adc_value[0]=0;
		adc_enable(ADC0);
		if((tmpv=v_check())>0){
			 return v_to_per(tmpv);
		}else{
			 return FALSE;
		}	
}
uint8_t BT_recvdatadec(uint8_t *u2rev)
{
	uint8_t revlen=0;
	     //if(fmrvpkg_cnt==35)
		  //debug_printf("\r\nno35");
	//if(USART_RX_STA&0x8000){
      if(u2rev[0]!=0xfd&&u2rev[1]!=0xdf){
				  return FALSE;
			}
     revlen=u2rev[2];
		 if(revlen!=(USART_RX_STA&0xFFF)){
			 return FALSE;
		 }
		 if(u2rev[revlen-1]!=0x55&&u2rev[revlen-2]!=0xaa){
			 	 return FALSE;
		 }
		if(checksum(&u2rev[5],revlen-7)!=(((uint16_t)u2rev[3])<<8)+u2rev[4]){
			 return FALSE;
		}
		//while(--revlen)
		//debug_printf("\r\nbtrv=%02x",*(u2rev++));
		uint8_t v_value=0;
		uint8_t vtmp[3]={0};
		static uint16_t udfwvtmp=0;
		static uint16_t fmrvpkg_cnt=0;
		static uint32_t fmcalpkgsz=0,fmpkgsz=0;
		//static uint16_t kfwsztmp=0;
										   static DEV_FTCODE devcd;
								   static NXP_DEVTPE_ENUM  devtyp;
		uint8_t tmpwsz=0,tmpwsz2=0;
		uint32_t tmp=0;
		uint32_t  tmpadr=0;
    switch(u2rev[5]){
			 case(0xd1):{
				 switch(u2rev[6]){
					 case(0xa1):      //Ê¶±ðidÃüÁî
						  BT_rdcmd_exe(0xa1);					    
					   break;
					 case(0xa2):      //±à¼­¶ÁÈ¡
						  BT_rdcmd_exe(0xa2);
					    
					  break;
					 case(0xb1):      //È«Ð´ÃüÁî
						 debug_printf("\r\nbtrv="); 
						 while(--revlen)
		         debug_printf("%02x",*(u2rev++)); 
					  break;
					 	case(0xc1):      //requir v 
						  //v_value=v_check();
			         v_value=BATV_swtrigcheck();
						   if(v_value<3)v_value=0;
							 BT_send(&v_value,1,0xd2,u2rev[6]);
								v_value=0;						 
					  break;
						case(0xc2):      //fmpakg version chk
						 tmpadr=*(uint32_t*)(FWCURVFLSADR);
	           vtmp[0]=(tmpadr&0xFF00)>>8;
						 vtmp[1]=tmpadr&0xFF;
						if(*(uint32_t*)(FWCURRNFLSADR)==FLS_SEC1_ADR){
							 vtmp[2]=0x0A;
						}else if(*(uint32_t*)(FWCURRNFLSADR)==FLS_SEC2_ADR){
							vtmp[2]=0x0B;
						}						
						 BT_send(vtmp,3,0xd2,u2rev[6]);
					  break;
						case(0xfe):      //fmpakg updtae
							//debug_printf("\r\ndpars");
						  	timer4_start( );
						    cmdflg=2;
							if(BYTASSBL(&u2rev[7],4)==MD){
								  udfwvtmp=0;
                  fmrvpkg_cnt=0;
                  fmcalpkgsz=0;
                  fmpkgsz=0;
                  udfwvtmp=u2rev[11];
						      udfwvtmp=(udfwvtmp<<8)|u2rev[12];
                  fmpkgsz=BYTASSBL(&u2rev[13],3);	
                  if(*(uint32_t*)(FWCURRNFLSADR)==FLS_SEC1_ADR){
										 if(u2rev[17]!=0x0B){
											 v_value=0xe6;
											 BT_send(&v_value,1,0xd3,u2rev[6]);
											 	v_value=0x0B;
											BT_send(&v_value,1,0xd2,u2rev[6]);
											   return 0xe6;
										 }
									}else if(*(uint32_t*)(FWCURRNFLSADR)==FLS_SEC2_ADR){
										 if(u2rev[17]!=0x0A){
											 v_value=0xe6;
											BT_send(&v_value,1,0xd3,u2rev[6]);
											 v_value=0x0A;
											BT_send(&v_value,1,0xd2,u2rev[6]);
											   return 0xe6;
										 }
									}else{
										return FALSE;
									}										
							 }
              //if(!fmrvpkg_cnt){							
							//udfwvtmp=u2rev[9];
						  //udfwvtmp=(udfwvtmp<<8)|u2rev[10];
							//}
						  if(fmrvpkg_cnt||(fmu_verupdchk(udfwvtmp))){
                 if(fmrvpkg_cnt||(0x4FF<fmpkgsz&&fmpkgsz<0xffffff)){
									 tmpwsz=fm_get_pklen(NUNDEV,revlen,fmrvpkg_cnt);
									 	if(fmrvpkg_cnt<(fmpkgsz+16)/192)
											{
													if(fmrvpkg_cnt==0){
														if(tmpwsz!=176)
															return FALSE;
													}else{
														if(tmpwsz%192)
														return FALSE;
													}
											}
				           if(tmpwsz==0||tmpwsz%4){										    
                       if(tmpwsz==0)
												return FALSE;
											 if(tmpwsz%4)
												 tmpwsz2=tmpwsz/4+1;
	                     }else{
												 tmpwsz2=tmpwsz/4;
											 }												 
									 if(fmcalpkgsz<=fmpkgsz){
										 	 if(fmrvpkg_cnt==0){
												     nvic_irq_disable(EXTI0_IRQn);
												     nvic_irq_disable(EXTI5_9_IRQn);
												     nvic_irq_disable(EXTI10_15_IRQn);
                             exti_interrupt_disable(EXTI_1);												 
												 		 vTaskDelete(CardrdTask_Handler);
										         vTaskDelete(SLEEP_Task_Handler);
												    // vTaskDelete(RFCK_Task_Handler);
												    //vTaskDelete(POWER_M_Task_Handler);
											       pswlckflg=1;
		                         tmp=(uint32_t)(&u2rev[23]);
												     if(*(uint32_t*)(FWCURRNFLSADR)==FLS_SEC1_ADR)
		                         tmpadr=FLS_SEC2_ADR;
														 else
														 tmpadr=FLS_SEC1_ADR;
                             fmc_erase_pages(tmpadr,FLS_SEC_PGSZ);														 
		                         //dma_config(DMA0,DMA_CH1);
	                       }else if(fmrvpkg_cnt>0){
                                 tmp=(uint32_t)&u2rev[7];
                                 if(*(uint32_t*)(FWCURRNFLSADR)==FLS_SEC1_ADR)													 
		                             tmpadr=FLS_SEC2_ADR+fmcalpkgsz;
                                 else
                                 tmpadr=FLS_SEC1_ADR+fmcalpkgsz;																	 
	                          }
						WAGIN:			if(fw_dlfls(tmpadr,tmp,tmpwsz2)){
												      v_value=0xe4;
												      BT_send(&v_value,1,0xd3,u2rev[6]);
													    fmrvpkg_cnt++;							                
                            	fmcalpkgsz+=tmpwsz;
                               info_printf("\r\ncnt=%d,rvpksz=%d,fmsz=%d",fmrvpkg_cnt,fmcalpkgsz,fmpkgsz);
															if(fmcalpkgsz==fmpkgsz){
																     timer4_stop();
		                                 timer4_ov_interrupt_dis();
																     t2_tick_1s=0;
																     cmdflg=0;
                                       v_value=0xe5;
												              BT_send(&v_value,1,0xd3,u2rev[6]);
                                        																
                                     if(fm_set_swfwadr(fmcalpkgsz,udfwvtmp)==FALSE){
																			   return FALSE; 
																		 }
																		 //cmdflg=0;
																		 INTX_DISABLE( );
																		 NVIC_SystemReset();
																	 //}
															}else{
																			 v_value=0xe0;
												              BT_send(&v_value,1,0xd3,u2rev[6]);
																         return 0xe0;
																		 }														
											}else{
																v_value++;
														    if(v_value<=2)
														    goto WAGIN;
                                v_value=0;	
												return FALSE;
											}
										}
									 else{
										 fmcalpkgsz=0; 
										 fmrvpkg_cnt=0;
										 udfwvtmp=0;
										 fmpkgsz=0;
										 cmdflg=0;
										 v_value=0xe0;
										 BT_send(&v_value,1,0xd3,u2rev[6]);
										 return 0xe0;
									 }									 
								 }else{
									 udfwvtmp=0;
									 fmpkgsz=0;
                   return FALSE;
								 }
							}else{
								 udfwvtmp=0;
								return FALSE;
							}
					  break;
						case(0xbf):      //fmpakg version backup
             if(fm_backlv()){
							  INTX_DISABLE( );
						    NVIC_SystemReset();
						 }else{
							  return FALSE;
						 }							 						 
					  break;
					 case(0xf1):      //fmpakg version backup
						 timer4_start( );
					   cmdflg=1;
						 switch(krvstate){
							 case chekhd:{
						   if(BYTASSBL(&u2rev[7],4)==BINCD){
								  devtyp=NUNDEV;
								if((u2rev[11]!=0x31&&u2rev[11]!=0x32)||(u2rev[12]!=0x51&&u2rev[12]!=0x52&&u2rev[12]!=0x53&&
									  u2rev[12]!=0x54&&u2rev[12]!=0x20)){
										 return FALSE;
									}else{
										if(u2rev[11]==0x31)
										 devcd=GC_DEV;
										if(u2rev[11]==0x32)
										 devcd=NXP_DEV;	
									}
								  if(u2rev[12]==0x51||u2rev[12]==0x53){
                       if(u2rev[12]==0x51)
                        devtyp=PCF7945;
                       else
                         devtyp=PCF7953;												 
									}else if(u2rev[12]==0x52||u2rev[12]==0x54){
										   //udfwvtmp=0x02;
										   devtyp=PCF7952;
									}else if(u2rev[12]==0x20){
										   devtyp=OTHERDEV; 
										    fmpkgsz=0;
										 fmpkgsz=BYTASSBL(&u2rev[13],2);
										if(!kfw_pkgszchk(devtyp,fmpkgsz)){
										   fmpkgsz=0;
										  return FALSE;
									  }	
										
									}
								if(devtyp!=OTHERDEV){
									user_kdl_init();
#if !FCMD 
								 	uint8_t rddtype=0;
                  uint8_t result=0;									
						  if((result=key_id(devcd,devtyp,&rddtype))==FALSE){								
								 	v_value=0xe7;
									BT_send(&v_value,1,0xd3,u2rev[6]);
								  krvstate=chekhd;
								  return 0xe7;
							 }else{
								  if(result==TRUE){
								   if(devcd==GC_DEV&&rddtype!=devtyp){
										    v_value=0;
										 if(rddtype==PCF7945)
										   v_value=0x51;
										 if(rddtype==PCF7952)
										   v_value=0x52;
										 if(rddtype==PCF7953)
										   v_value=0x53;
										 BT_send(&v_value,1,0xd2,u2rev[6]);
										 return FALSE;
									}else{
												v_value=0xe4;
									     BT_send(&v_value,1,0xd3,u2rev[6]);
                       krvstate=brcv;
									}
								 }
                if(result==2){
									v_value=0xe8;
									BT_send(&v_value,1,0xd3,u2rev[6]);
								  krvstate=chekhd;
								  return 0xe8;
								}									
							 }
#endif
						 }else{
							 v_value=0xe4;
							 BT_send(&v_value,1,0xd3,u2rev[6]);
							 krvstate=brcv;
							    fmrvpkg_cnt=0;
                  fmcalpkgsz=0;
						 }
#if FCMD
       			 	v_value=0xe4;
							 BT_send(&v_value,1,0xd3,u2rev[6]);
							 krvstate=brcv;
#endif
						 }else{
							  return FALSE;
						 }
					 }
						   break;
							 case brcv:{
             	if(devtyp!=OTHERDEV&&BYTASSBL(&u2rev[7],4)==KMD){
                  fmrvpkg_cnt=0;
                  fmcalpkgsz=0;
                  fmpkgsz=0;
								  //udfwvtmp=0;
                  //udfwvtmp=u2rev[11];
						      //udfwvtmp=(udfwvtmp<<8)|u2rev[12];
								   //static DEV_FTCODE devcd;
								   //static NXP_DEVTPE_ENUM  devtyp;
								  //if(u2rev[7]!=0x31||(u2rev[8]!=0x51&&u2rev[8]!=0x52&&u2rev[8]!=0x53)){
									//	 return FALSE;
									//}
                  fmpkgsz=BYTASSBL(&u2rev[11],4);	
                  if(!kfw_pkgszchk(devtyp,fmpkgsz)){
										   fmpkgsz=0;
										  return FALSE;
									}	
                  if(u2rev[16]!=0x0e||u2rev[18]!=0x0e){
										   return FALSE;
									 }							
							  }						   
							     tmpwsz=fm_get_pklen(devtyp,revlen,fmrvpkg_cnt);
								//if(fmrvpkg_cnt==49){
								//	debug_printf("\r\ncnt=%d",fmrvpkg_cnt);
								//}
								      if(fmrvpkg_cnt<(fmpkgsz+16)/192)
											{
												if(devtyp==OTHERDEV){
												  if(tmpwsz%192)
														return FALSE;
												}else{
													if(fmrvpkg_cnt==0){
														if(tmpwsz!=176)
															return FALSE;
													}else{
														if(tmpwsz%192)
														return FALSE;
													}
												}
											}
									 	if(tmpwsz==0||tmpwsz%4){
											 if(tmpwsz==0)
												return FALSE;
											 if(tmpwsz%4)
												 tmpwsz2=tmpwsz/4+1;
	                     }else{
												 tmpwsz2=tmpwsz/4;
											 }
											 
										if(fmcalpkgsz<=fmpkgsz){
												 if(fmrvpkg_cnt==0){
													   nvic_irq_disable(EXTI0_IRQn);
												     nvic_irq_disable(EXTI5_9_IRQn);
												     nvic_irq_disable(EXTI10_15_IRQn);
													   exti_interrupt_disable(EXTI_1);
                             	//vTaskSuspend(CardrdTask_Handler);
												     	//vTaskSuspend(SLEEP_Task_Handler);
													   // vTaskSuspend(RFCK_Task_Handler);
													 	  //vTaskSuspend(POWER_M_Task_Handler);
												 		 //vTaskDelete(CardrdTask_Handler);
										         //vTaskDelete(SLEEP_Task_Handler);
												     //vTaskDelete(POWER_M_Task_Handler);
											       pswlckflg=1;
													  if(devtyp!=OTHERDEV)
		                         tmp=(uint32_t)(&u2rev[23]);
														else
														 tmp=(uint32_t)&u2rev[7];
		                         tmpadr=BIN_EEFLSBR_START_ADDR;
                             fmc_erase_pages(tmpadr,KFW_PGSZ);														 
		                         //dma_config(DMA0,DMA_CH1);
	                       }else if(fmrvpkg_cnt>0){
													      
                                 tmp=(uint32_t)&u2rev[7];	
																 
		                             tmpadr=BIN_EEFLSBR_START_ADDR+fmcalpkgsz;																	 
	                          }
					WAGIN1:			if(fw_dlfls(tmpadr,tmp,tmpwsz2)){
												      v_value=0xe4;
												      BT_send(&v_value,1,0xd3,u2rev[6]);
													    fmrvpkg_cnt++;							                
                            	fmcalpkgsz+=tmpwsz;
                              // info_printf("\r\ncnt=%d,rvpksz=%d,fmsz=%d",fmrvpkg_cnt,fmcalpkgsz,fmpkgsz);
															if(fmcalpkgsz==fmpkgsz){
																    timer4_stop();
		                                timer4_ov_interrupt_dis();
		                                t2_tick_1s=0;
																    cmdflg=0;
																uint32_t ERdlflsadr=0;
																    if(devtyp==PCF7945||devtyp==PCF7953||devtyp==PCF7952){
																			ERdlflsadr=BIN_EEFLSBR_START_ADDR+1028;
																		}/*else if(devtyp==PCF7952){
																			ERdlflsadr=BIN_EEFLSBR_START_ADDR+516;
																		}*/else if(devtyp==OTHERDEV){
																			ERdlflsadr=fmcalpkgsz;
																		}
																			
																		 if(fm_burd(devcd,devtyp,/*fmcalpkgsz,*/ERdlflsadr,BIN_EEFLSBR_START_ADDR)
															            ){
                                       v_value=0xe5;
												              BT_send(&v_value,1,0xd3,u2rev[6]);																			 
																		 }else{
																			 v_value=0xe0;
												              BT_send(&v_value,1,0xd3,u2rev[6]);
																		 }
																		krvstate=chekhd;
                                   nvic_irq_enable(EXTI0_IRQn,6U, 2U);																		 
                                   nvic_irq_enable(EXTI5_9_IRQn,6U, 2U);
												           nvic_irq_enable(EXTI10_15_IRQn,6U, 2U);
																		 exti_interrupt_enable(EXTI_1);
																		//vTaskResume(CardrdTask_Handler);
                                    //vTaskResume(SLEEP_Task_Handler);
																		 //vTaskResume(RFCK_Task_Handler);
                                    //vTaskResume(POWER_M_Task_Handler);
                                    pswlckflg=0;																		 
															}														
											}else{
																v_value++;
														    if(v_value<=2)
														    goto WAGIN1;
                                //v_value=0;	
												return FALSE;
											}
											
										}else{
										 fmcalpkgsz=0; 
										 fmrvpkg_cnt=0;
										// udfwvtmp=0;
										 fmpkgsz=0;
										 krvstate=chekhd;
											cmdflg=0;
										 	v_value=0xe0;
										 BT_send(&v_value,1,0xd3,u2rev[6]);
										 return 0xe0;
									 }
								 }
							  break;
							default: 
								v_value=0xee;
								BT_send(&v_value,1,0xd3,u2rev[6]);
						   return 0xee; 
									 }												 
					  break;
					 default: 
						return FALSE; 
				 }
					 
			 } 
			 break;	
	 default:
        return FALSE;				 
    }	
  //}	
return TRUE;	
}
