#include "headfile.h"

extern volatile uint8_t pswlckflg;
extern volatile uint8_t  cmdflg;
extern const unsigned char BAT_EMPT_ICON_BMP[];
extern const unsigned char BT_ICON_BMP[];
extern volatile uint8_t exti_keystartflg;
SemaphoreHandle_t KeyPSPhore;
uint8_t volatile slpflg=0;
extern volatile enum {
		chekhd,
	  brcv,
	}krvstate;
static bool BT_disc(void)
{
	sprintf((char *)sendbuf, "AT+DISC\r\n");
	sprintf((char *)ackbuf, "OK+DISC\r\n");
		if(BT_u2sndr_cmd(sendbuf,ackbuf)){
		 return TRUE;
	}else{
		 return FALSE;
	}
}
bool  BATV_scchek(void)
{
	uint8_t batperv=0;
	batperv=timer_BATV_chek(ADC0,ADC0_1_EXTTRIG_REGULAR_T1_CH1);
	if(batperv<=5){
		OLED_Clear();
		OLED_DrawBMP(50,0,58+50,4,BAT_EMPT_ICON_BMP);
		for(uint8_t i=6;i>1;i--){
		delay_us(1000000);
		if(i%2==0){
		  OLED_Display_Off();
		}else{
			OLED_Display_On();
		}
	  }
		OLED_Clear();
		OLED_Display_On();
		return FALSE;
  }
	return TRUE;	
}
static void ent_sleep_mode(void)
{
	 slpflg=1;
	 pmu_deinit();
	 pmu_to_deepsleepmode(PMU_LDO_LOWPOWER,WFI_CMD);
	 SystemInit();
	 SystemCoreClockUpdate();
}
static  void chek_pw(void)
{
	  	uint8_t v_value=0;
	 	v_value=BATV_swtrigcheck();
	taskENTER_CRITICAL(); 
	OLED_DrawBMP(50,0,58+50,4,BAT_EMPT_ICON_BMP);
	OLED_drawline(55,5,55+(v_value*42/100),28,1);
 taskEXIT_CRITICAL(); 	
}
void SLEEP_task(void *pvParameters)
{	
	 sleep_mag();
}
void sleep_mag(void)
{
	//timer_enable(TIMER1);
	BaseType_t err=pdFALSE;
	vSemaphoreCreateBinary(KeyPSPhore);
	xSemaphoreTake(KeyPSPhore,portMAX_DELAY);
	while(1){
  //if(!usbpow_sig_level){	
   err=xSemaphoreTake(KeyPSPhore,360000);
		//vTaskDelay(60000);
	  if(err==pdFALSE){
		   //if(mincnt==0){
				 timer_disable(TIMER1);
				 adc_disable(ADC0);
			  // timer_disable(TIMER7);
		    // exti_interrupt_disable(EXTI_1);
			   //rfckstart=0;	
				 OLED_Display_Off( );
			   //LCD_VCC_OFF;
			   ICD_5V_OFF;
	       RFCHK_5V_OFF;
				 if(BT_LKSTA){
					 if(BT_disc()){
					 }else{
						 BT_disc();
					 }
				 }
				 gpio_init(GPIOA,GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
				 gpio_bit_reset(GPIOA,GPIO_PIN_2);
				 gpio_bit_reset(GPIOC,GPIO_PIN_3);
				 BT_VCC_OFF;
				 ent_sleep_mode();
         OLED_Display_On();
				 gpio_init(GPIOA,GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
				 if(exti_keystartflg){
					 OLED_Clear();
				   OLED_ShowString(0,0,DIS_V,16,16);
					 exti_keystartflg=0;
					 //delay_us(100000);
				 }
				  BT_VCC_ON;
				 //LCD_VCC_ON;
				 BT_MOD_reset();
		     adc_enable(ADC0);
		     TCNT2_V=0;
		     timer_enable(TIMER1);
				 slpflg=0;
         debug_printf("\r\ndwkup");				 
			  }
		 //  else if(mincnt==1){
		 	//  OLED_Display_Off( );
		   // poff_sigal_lock;   //close set
			// }
			// mincnt++;
	 //}else{
		// mincnt=0;
	// }
 // }
 //vTaskDelay(10);	 
	}	
}
#define  TO   110
void POWER_M_task(void *pvParameters)
{	
	power_mag();	
}
void power_mag(void)
{
	static uint8_t bt_linked=0xFF;
	uint8_t v_value=0;
	uint8_t clsscnt=0;
	while(1){
		 //if(eTaskGetState(RFCK_Task_Handler)==eSuspended)
			 // rfckstart=0;		
			if(!pswlckflg){
				//clsscnt=TO;
			 if(KEY_Scan(1)==KEYSTART_SIG_PRES&&(++clsscnt>TO)){
				 clsscnt=0;
				 LCD_VCC_OFF;
			 if(poctrl_sig_level){
         fls_infs( );				 
				poff_sigal_lock;
				//pswflg=0;
				while(1);
			  }
		  }else if(keystart_sigal_level&&(clsscnt<=TO)){
				clsscnt=0;
			}
			 //if(KEY_Scan(0)==KEYSTART_SIG_PRES)
			 //{
				// if(!poctrl_sig_level){	
				// pon_sigal_lock;			   
         //}					 
			 //}
			}
  if((t2_tick_1s>5)&&(cmdflg==1||cmdflg==2)){
		 t2_tick_1s=0;
		timer4_ov_interrupt_dis();
		timer4_stop();
		if(cmdflg==1){
			krvstate=chekhd;
		}
		 v_value=0xe0;
		BT_send(&v_value,1,0xd3,0);
  }		
	if(usbpow_sig_level){
		if(!(TIMER_CTL0(TIMER1)&(uint32_t)TIMER_CTL0_CEN)){
     adc_timer1trig_confg( );
		}
	//	debug_printf("\r\nTCNT2_V=%d",TCNT2_V);
   if(!BATV_scchek()){
		 //delay_us(1000000);
		  OLED_Clear();
		  OLED_ShowString(0,0,"NO POWER",16,8);
		  BT_VCC_OFF;
		  fls_infs( );
		  delay_us(2000000);
			OLED_Display_Off( );
		 	poff_sigal_lock;   //close set
      while(1);		 
      }
	   }else{
			 if((TIMER_CTL0(TIMER1)&(uint32_t)TIMER_CTL0_CEN)){
			     adc_disable(ADC0);
			     timer_disable(TIMER1);
			 }
		 }
		 if(BT_LKSTA!=bt_linked){
			 taskENTER_CRITICAL();	 
			OLED_Clear();
				OLED_DrawBMP(0,0,18,4,BT_ICON_BMP);
			if(BT_LKSTA){
		    OLED_DrawBMP(17,2,16+17,2+2,BT_CNED_ICON_BMP);
	    }else{
	 	    OLED_DrawBMP(17,2,16+17,2+2,BT_DISCED_ICON_BMP);
 	    }
				 taskEXIT_CRITICAL();
			chek_pw( );
		  bt_linked=BT_LKSTA;
			if(BT_LKSTA){
			 xSemaphoreGive(KeyPSPhore);
		 }
   }
//	if(rfckstart){
     // timer_enable(TIMER7);
		//  exti_interrupt_enable(EXTI_1);
 //  }
	// portBASE_TYPE uxHighWaterMark;
   //uxHighWaterMark=uxTaskGetStackHighWaterMark(POWER_M_Task_Handler);
   //debug_printf("\r\npmtasksz=%d",(uint16_t)uxHighWaterMark);
   //portBASE_TYPE uxHighWaterMark2;
   //uxHighWaterMark2=uxTaskGetStackHighWaterMark( SLEEP_Task_Handler );
   //debug_printf("\r\nslptasksz=%d",(uint16_t)uxHighWaterMark2);
		 vTaskDelay(10);	
 }
   	
}


