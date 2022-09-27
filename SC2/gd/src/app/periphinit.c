#include "headfile.h"
#include "bmp.h"
extern const unsigned char BAT_V_ICON_BMP[];
uint8_t temp[4]={0};
static void user_gpio_init(void)
{
	//GPIO_InitTypeDef  GPIO_InitStructure;
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_AF);
	rcu_periph_clock_enable(RCU_GPIOD);
	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP , ENABLE);       //开启SWD，失能JTAG
	  //uart0 gpio init
		//USART1_TX   GPIOA.9,USART2_TX  PA.2,uart4_tx pc10
  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9|GPIO_PIN_2);
	gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
	//USART1_RX	  GPIOA.10,USART2_RX PA.3，usart4_RX pc11 
   gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10|GPIO_PIN_3);
	//gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
	gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
	  //keypad gpio init
	gpio_init(GPIOA,GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, CRDKYIN|PGDKYIN|PCF7991DOUT_IN);//GPIOA_Pin_15,GPIOA_Pin_0,PA.8
	gpio_init(GPIOC,GPIO_MODE_IPU, GPIO_OSPEED_50MHZ,RCCKYIN); //GPIOC_Pin_5
	//pcf7991 gpio
	gpio_init(GPIOC,GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_4|GPIO_PIN_9|PCF7991_SCLK_OUT|GPIO_PIN_3|TMS3705OCT|LCD_BL_PC7);//GPIOC_pin_4:L7991,H3705，//GPIOC_Pin_12,GPIOC_Pin_11,BTrstpin
	//gpio_init(GPIOC,GPIO_MODE_OUT_OD, GPIO_OSPEED_50MHZ,GPIO_PIN_9);
	gpio_bit_reset(GPIOC,GPIO_PIN_4);
	gpio_bit_reset(GPIOC, GPIO_PIN_9);
	//power control pin PB14,PB12 //usb chek
	gpio_init(GPIOB,GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_14|GPIO_PIN_9);//PB.14  key_pws //PB.9 usbchek in
	gpio_init(GPIOB,GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);//PB.12  pws
	gpio_init(GPIOB, GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,ICID_5V_CTRL_PB13| RFCHK_5V_CTRL_PB15);
	//usb chek
	//gpio_init(GPIOB,GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_9);//PB.9 usbchek in
	//voltage check gpio
	gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ,GPIO_PIN_1); 
		  //TMS3705 gpio
	gpio_init(GPIOC,GPIO_MODE_IPU, GPIO_OSPEED_50MHZ,TMS3705IN); //GPIOC_Pin_0
	gpio_init(GPIOC,GPIO_MODE_IPD, GPIO_OSPEED_50MHZ,GPIO_PIN_2); //GPIO_Pin_2:btstate_input
  //gpio_init(GPIOC,GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,TMS3705OCT);
	//ST7735S LCD gpio
	//gpioB0~15for lcd_d0~d7
	//gpio_init(GPIOB,GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,PB0_7);
	//gpio_bit_set(GPIOB,PB0_7);
	//gpio_init(GPIOC,GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_4/*|LCD_RD_PC6|LCD_WR_PC7|LCD_RS_PC8|LCD_CS_PC9|LCD_BL_PC10*/);//GPIOC_pin_4:L7991,H3705，GPIOC_Pin_6~10
	gpio_init(GPIOD,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,PCF7991DIN_OUT);
	    /* connect PB6 to I2C0_SCL */
    /* connect PB7 to I2C0_SDA */		
  gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_10 | GPIO_PIN_11);
	//delay_us(10000);
  gpio_bit_set(GPIOC,GPIO_PIN_3);
	Key_EXTI_Init();
	LCD_BL_CLR;
	//delay_us(10);
 // gpio_bit_reset(GPIOC,GPIO_PIN_3);	//reset bt
	//91&3705rayctrlio
}
static void Get_ChipID(void)
{
    uint32_t temp0,temp1,temp2;
temp0 = *(__IO uint32_t*)(0x1FFFF7E8);    //产品唯一身份标识寄存器（96位）
    temp1 = *(__IO uint32_t*)(0x1FFFF7EC);
temp2 = *(__IO uint32_t*)(0x1FFFF7F0);
   temp1=temp0^temp1;
   temp2=temp2^temp1;
   temp[0]=temp2&0xFF;
   temp[1]=(temp2>>8)&0xFF;
   temp[2]=(temp2>>16)&0xFF;
   temp[3]=(temp2>>24)&0xFF;	
}
static void pmu_init(void)
{
 rcu_periph_clock_enable(RCU_PMU);	
}
void periph_init(void)
{
	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
	delay_init();
	Get_ChipID( );
	user_gpio_init();
	//user_kdl_init();
	if(!keystart_sigal_level){
		pon_sigal_lock;
	}
	user_timer_init();
	uart_init(USART0,115200);	 	//串口初始化为115200
	uart_init(USART1,115200);
  if(!BT_UARTIF_init( )){
		info_printf("\r\nBT init fail\r\n");
	}else{
		if(!set_BTname()){
			info_printf("\r\nBTname set fail\r\n");
		}
		nvic_irq_enable(USART1_IRQn,7,2);
	}
	//user_lcd_init( );	//LCD初始化	
	i2c_init(I2C1);
	oled1306_init();
	rf_chk_init();
//	lfe3_init();
  //swd_cls( );	
	//pcf7991_init();	 
	//tms3705_init();	
	//#if _INIT_TS_TAG
	//tms37145_get_data_init();
	//tms37145_fast_read_init();
  //tms37206_fast_read_init();
	//tag64_fast_read_init();
	//ts8x_init_tag_init();
	//ts0d_init_tag63_1_init();
	//ts0d_init_tag63_2_init();
	//ts0d_init_tag63_init();
	//ts0d_init_tag4d_init();
	//ts0d_init_tag64_init();
	//ts0d_unlock_tag_init();
	//ts32_init_tag_init();
	//ts48_init_tag_init();
	//em4170_fast_read_init();
	//ts06new_init_tag_init();
	//ts46_init_tag_init();
	//pcf7936_fast_read_init();
	//ts42_init_tag_init();
	//tag4c_fast_read_init();
	//#else
	//lcd_print_str(0, 0, str_r_is_press_read, 16);
	//lcd_print_str(0, 1, str_c_is_press_copy, 16);
	//delay_ms(4000);
	//lcd_print_ready_press();
	//#endif
	
	//__enable_interrupt();
	dma_config(DMA0,DMA_CH0);
	adc_config( );
	pmu_init();
	pcf7936_t_charge = PCF7936_CHARGE_TIME;
	g_pcf7991_p0_pcf7936 = PCF7991_PAGE0_PCF7936_NEW_BOARD;
	ICD_5V_OFF;
  RFCHK_5V_OFF;
	info_printf("\r\ninit ok!");
  /* if(10<v_value<=25)	
      OLED_DrawBMP(45,3,2,32,BAT_V_ICON_BMP);			
	 else if(25<v_value<=50){
			OLED_DrawBMP(45,3,2,32,BAT_V_ICON_BMP);
		  OLED_DrawBMP(56,3,2,32,BAT_V_ICON_BMP);
	    }
	 else if(50<v_value<=75){
			OLED_DrawBMP(45,3,2,32,BAT_V_ICON_BMP);
		  OLED_DrawBMP(56,3,2,32,BAT_V_ICON_BMP);
		  OLED_DrawBMP(67,3,2,32,BAT_V_ICON_BMP);
	   }
	 else if(75<v_value<=100){
			OLED_DrawBMP(45,3,2,32,BAT_V_ICON_BMP);
		  OLED_DrawBMP(56,3,2,32,BAT_V_ICON_BMP);
		  OLED_DrawBMP(67,3,2,32,BAT_V_ICON_BMP);
		  OLED_DrawBMP(78,3,2,32,BAT_V_ICON_BMP);
	  }*/
}


