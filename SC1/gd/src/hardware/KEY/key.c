#include "headfile.h"
								    

#define  KEY_EXTI_LINE1    (EXTI_15)
#define  KEY_EXTI_LINE2    (EXTI_5)
#define  KEY_EXTI_LINE3    (EXTI_0)
#define  KEY_EXTI_LINE4    (EXTI_14)
static uint8_t  keypre=0;
volatile uint8_t rfckstart=0;
volatile uint8_t exti_keystartflg=0;
extern SemaphoreHandle_t KeyPSPhore;
SemaphoreHandle_t KeyprePhore;
void Key_EXTI_Init(void)
{
	
  gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOC, GPIO_PIN_SOURCE_5);  
  gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_15);
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_0);
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_14);
	  nvic_irq_enable(EXTI0_IRQn,6U, 2U);
    nvic_irq_enable(EXTI5_9_IRQn,6U, 2U); 
    nvic_irq_enable(EXTI10_15_IRQn,6U, 2U);  
  exti_init(EXTI_0, EXTI_INTERRUPT, EXTI_TRIG_FALLING);	
	exti_init(EXTI_5, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
	exti_init(EXTI_15, EXTI_INTERRUPT,EXTI_TRIG_FALLING);
	exti_init(EXTI_14, EXTI_INTERRUPT,EXTI_TRIG_FALLING);
	exti_interrupt_flag_clear(EXTI_0);
	exti_interrupt_flag_clear(EXTI_5);
	exti_interrupt_flag_clear(EXTI_15);
	exti_interrupt_flag_clear(EXTI_14);
}
uint8_t KEY_Scan(uint8_t mode)
{	 
	static uint8_t key_up=1;
	if(mode)key_up=1;  	  
	if(key_up&&(keypad_read_level==0||keypad_write_level==0|| keystart_sigal_level==0||keypad_FCHK_level==0))
	{
		delay_us(10000);
		key_up=0;
		if(keypad_read_level==0)return KEYPAD_RD_PRES;
		else if(keypad_write_level==0)return KEYPAD_WR_PRES;
		else if(keypad_FCHK_level==0)return KEYPAD_FCHK_PRES;
		else if(keystart_sigal_level==0)return KEYSTART_SIG_PRES;
	}else if(keypad_read_level==1&&keypad_write_level==1&&keystart_sigal_level==1)key_up=1; 	    
 	return 0;
}
void EXTI10_15_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken1=pdFALSE,xHigherPriorityTaskWoken2=pdFALSE;
		uint32_t  retur;
	  //uint8_t  kpre_tmp=0;
	  retur=taskENTER_CRITICAL_FROM_ISR();
	if (RESET != exti_interrupt_flag_get(KEY_EXTI_LINE1)) {
		  exti_interrupt_flag_clear(KEY_EXTI_LINE1);
		  debug_printf("\r\nrkeyd");
		if(!slpflg)
		xSemaphoreGiveFromISR(KeyprePhore,&xHigherPriorityTaskWoken1);
		//vTaskNotifyGiveFromISR(CardrdTask_Handler,&xHigherPriorityTaskWoken1);
	  }
		if (RESET != exti_interrupt_flag_get(KEY_EXTI_LINE4)) {
		  exti_interrupt_flag_clear(KEY_EXTI_LINE4);
			exti_keystartflg=1;
	  }
		 //  delay_ms(10);
		 // if(keypad_read_level){
		  //keypre=KEYPAD_RD_PRES;
		//kpre_tmp=KEY_Scan(0);
		 if(slpflg)
		  xSemaphoreGiveFromISR(KeyPSPhore,&xHigherPriorityTaskWoken2);
		  portYIELD_FROM_ISR((xHigherPriorityTaskWoken1|xHigherPriorityTaskWoken2));
			//}
		taskEXIT_CRITICAL_FROM_ISR( retur );
}
void EXTI0_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken1=pdFALSE,xHigherPriorityTaskWoken2=pdFALSE;
	uint32_t  retur;
	  retur=taskENTER_CRITICAL_FROM_ISR();
		if (RESET != exti_interrupt_flag_get(KEY_EXTI_LINE3)) {
					   exti_interrupt_flag_clear(KEY_EXTI_LINE3);	
			debug_printf("\r\nfckeyd");
			 //keypre=KEYPAD_WR_PRES;
			if(!slpflg)
       xSemaphoreGiveFromISR(KeyprePhore,&xHigherPriorityTaskWoken1);
			//vTaskNotifyGiveFromISR(CardrdTask_Handler,&xHigherPriorityTaskWoken1);
			 else
       xSemaphoreGiveFromISR(KeyPSPhore,&xHigherPriorityTaskWoken2);			
       portYIELD_FROM_ISR((xHigherPriorityTaskWoken1|xHigherPriorityTaskWoken2));			
	}
		taskEXIT_CRITICAL_FROM_ISR( retur );
	
}
void EXTI5_9_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken1=pdFALSE,xHigherPriorityTaskWoken2=pdFALSE;
	uint32_t  retur;
	  retur=taskENTER_CRITICAL_FROM_ISR();
		if (RESET != exti_interrupt_flag_get(KEY_EXTI_LINE2)) {
					   exti_interrupt_flag_clear(KEY_EXTI_LINE2);	
			debug_printf("\r\nwkeyd");
			 //keypre=KEYPAD_WR_PRES;
			if(!slpflg)
       xSemaphoreGiveFromISR(KeyprePhore,&xHigherPriorityTaskWoken1);
			//vTaskNotifyGiveFromISR(CardrdTask_Handler,&xHigherPriorityTaskWoken1);
			 else
       xSemaphoreGiveFromISR(KeyPSPhore,&xHigherPriorityTaskWoken2);			
       portYIELD_FROM_ISR((xHigherPriorityTaskWoken1|xHigherPriorityTaskWoken2));			
	}
		taskEXIT_CRITICAL_FROM_ISR( retur );
	
}
void cardrd_task(void *pvParameters)
{
  keypad_event_check();	
}
void keypad_event_check(void)
{
#define TRY_TIME	(2)	// in order to read stable
uint8_t i,d7991_find_ok = FALSE,d3705_find_OK = FALSE;
uint8_t ttf,scantimes=0; 
	vSemaphoreCreateBinary(KeyprePhore);
  xSemaphoreTake(KeyprePhore,portMAX_DELAY);
while(1){
//keypre=KEY_Scan(0);
	//debug_printf("\r\ntask1");
if(KeyprePhore!=NULL&&xSemaphoreTake(KeyprePhore,portMAX_DELAY)==pdTRUE/*ulTaskNotifyTake(pdTRUE,portMAX_DELAY)*/){
	keypre=KEY_Scan(0);
	d7991_find_ok = FALSE;
	d3705_find_OK = FALSE;
	//	vTaskSuspend(RFCK_Task_Handler);
switch(keypre){
  case KEYPAD_RD_PRES:{
		if(rfckstart){
			rfckstart=0;
			RFCHK_5V_OFF;
				timer_disable(TIMER7);
		   exti_interrupt_disable(EXTI_1);
		}
		  ICD_5V_ON;

	pcf7991_init();	 
	tms3705_init();			
			  oled_print_reading(); 
				 //DEV_SEL_7991;
		     //delay_us(1000);
  			/* check ASK tag talk frist */
		//debug_printf("\r\ntask1dbg");
		scantimes=2;		
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
			d7991_find_ok = em4170_keypad_interaction(&em4170_tag1);
		}
		if(d7991_find_ok){break;}
	}
				/* new(48) */
		if(!DEV_SELPIN_level){
		for (i = 0; (i<TRY_TIME)&&(!d7991_find_ok); i++) {
			new48_find_tag(&em4170_tag1);
			d7991_find_ok = em4170_keypad_interaction(&em4170_tag1);
		}
			if(d7991_find_ok){break;}
	 }

	}

	/* ASK reader talk first or TI FSK */
	else {

		if(DEV_SELPIN_level){
  	for (i = 0; (i<TRY_TIME)&&(!d3705_find_OK); i++) {
			tms37145_find_tag(&tms37145_tag1);
			d3705_find_OK = tms37145_keypad_interaction(&tms37145_tag1);
		}
			if(d3705_find_OK){break;}
	}

	if(!DEV_SELPIN_level){
   ts06_pcf7936typeinit_tag(&pcf7936_tag1/*,pcf7936_yes,nocrty*/);
		for (i = 0; (i<TRY_TIME)&&(!d7991_find_ok); i++) {
			if(pcf7936_find_tag(&pcf7936_tag1)){
			d7991_find_ok = pcf7936_keypad_interaction(&pcf7936_tag1);
			}
		}
		if(d7991_find_ok){break;}
	}
		}
	  if(DEV_SELPIN_level)
			   DEV_SEL_7991;
	  else
			  DEV_SEL_3705;
        delay_us(1000);		
	}
	   if (!d7991_find_ok&&!d3705_find_OK)
		    oled_print_ng_press();
  }
	break;
	case KEYPAD_WR_PRES:		
	  {
		if(rfckstart){
			rfckstart=0;
			RFCHK_5V_OFF;
				timer_disable(TIMER7);
		   exti_interrupt_disable(EXTI_1);
		}
         ICD_5V_ON;
			delay_us(20);
	     pcf7991_init();	 
	    tms3705_init();		
			oled_print_writing();
		  break;
	  }
	case KEYPAD_FCHK_PRES:
	  {
			//RFCHK_5V_ON;
			delay_us(20);
			oled_print_RFrate();			
		   //rfckstart=1;
		  break;
	  }
	default:
		break;
  }
 if(ICD_5V_CTRL_PIN_LEVEL)
	   ICD_5V_OFF;
 }
//debug_printf("\r\ntask1end");
 // if(keypre&&(KeyPSPhore!=NULL))
	//{
	//	xSemaphoreGive(KeyPSPhore);
		//portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//	}
// vTaskDelay(10);
 }

}
