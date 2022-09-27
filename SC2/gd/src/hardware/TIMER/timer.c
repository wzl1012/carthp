#include "headfile.h"

//uint8_t t1_tick_16ms; //t1_tick_2s;
volatile uint8_t t2_tick_16ms=0,t2_tick_1s=0;


//void TIM1_IRQHandler(void)
//{
	//g_sreg = psrg;
	//TCNT0 = 0x06;
   // if(SET ==TIM_GetITStatus(TIM1,TIM_IT_Update)){
        /* clear channel 0 interrupt bit */
  //      TIM_ClearITPendingBit(TIM1,TIM_IT_Update);
  //  }
//	++t1_tick_16ms;
//	if (t1_tick_16ms >= 125) {
	//	t1_tick_16ms = 0;
	//	++t1_tick_2s;
	//	if (t1_tick_2s >= (LFE3_INIT_SECOND/2)) {
	//	   TIM_Cmd(TIM1, DISABLE); 
	//		TIM_ITConfig(TIM1,TIM_IT_Update,DISABLE);
	//	}
	//}

	//SREG = g_sreg;
//}
 void TIMER4_IRQHandler(void)
{
	//g_sreg = SREG;
//	TCNT2 = 0x06;
    if(SET == timer_interrupt_flag_get(TIMER4,TIMER_INT_FLAG_UP)){
        /* clear channel 0 interrupt bit */
        timer_interrupt_flag_clear(TIMER4,TIMER_INT_FLAG_UP);
    }
	++t2_tick_16ms;
	if (t2_tick_16ms >= 62) {
		t2_tick_16ms = 0;
		++t2_tick_1s;
		//lcd_print_bcd(6, 1, t2_tick_1s >> 1);		// 665 us
	}

	//SREG = g_sreg;
}

void user_timer_init(void)
{
	    timer_oc_parameter_struct timer_ocintpara;
	    timer_parameter_struct timer_initpara;
	  rcu_periph_clock_enable(RCU_TIMER1);
    rcu_periph_clock_enable(RCU_TIMER2);
	  rcu_periph_clock_enable(RCU_TIMER4);
	  timer_deinit(TIMER1);
	  timer_deinit(TIMER2);
	  timer_deinit(TIMER4);
    /* initialize TIMER init parameter struct */
    timer_struct_para_init(&timer_initpara);
    /* TIMER1 configuration */
    timer_initpara.prescaler         = 6912-1;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 250-1;        //16ms 
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_init(TIMER4, &timer_initpara);
    timer_init(TIMER2, &timer_initpara);
		 
		 timer_initpara.prescaler         = TIM1_PSC;
		 timer_initpara.period            = TIM1_PERID;  //50s
		 timer_init(TIMER1, &timer_initpara);
	   //timer_init(TIMER1, &timer_initpara);
		    /* CH0 configuration in PWM mode1 */
     timer_ocintpara.ocpolarity  = TIMER_OC_POLARITY_HIGH;
     timer_ocintpara.outputstate = TIMER_CCX_ENABLE;
     timer_channel_output_config(TIMER1, TIMER_CH_1, &timer_ocintpara);

     timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_1,399);
     timer_channel_output_mode_config(TIMER1, TIMER_CH_1, TIMER_OC_MODE_PWM0);
     timer_channel_output_shadow_config(TIMER1, TIMER_CH_1, TIMER_OC_SHADOW_DISABLE);
		 timer_auto_reload_shadow_enable(TIMER1);
		 //timer_primary_output_config(TIMER1, ENABLE);
	   //timer_flag_clear(TIMER4,TIMER_FLAG_UP); 
		//timer_interrupt_enable(TIMER3, TIMER_INT_UP );
		t2_tick_16ms = 0;
	  t2_tick_1s = 0;
	  //timer_enable(TIMER3);
		//nvic_config( );
}
void timer_start(uint32_t timer_periph,uint16_t Prescaler, uint32_t TIM_PSCReloadMode)
{
	timer_prescaler_config(timer_periph,Prescaler,TIM_PSCReloadMode);
	//TIMER_CAR(timer_periph)=tscnt;
	timer1_clear_ov();
	//debug_printf("\r\n t2cnt=0x%x",TCNT_V);
	timer_enable(timer_periph);
}
void timer1_delay_us(uint16_t us)
{
	INTX_DISABLE();
	timer1_stop();
	timer1_clear_ov();
	TCNT1 = (/*65536 - */(us<<1)-1) /*+ 10)*/;		// subtract 5 us, see disassembler
	timer1_start_8();
	while (timer1_ov_flag == 0);
	INTX_ENABLE();
	timer1_stop();
	timer1_clear_ov();
}
void  timer4_start(void)
{
	t2_tick_16ms = 0;
	t2_tick_1s = 0;
	TCNT_V=0;
	timer_flag_clear(TIMER4,TIMER_FLAG_UP); 
	nvic_irq_enable(TIMER4_IRQn,9U, 2U);
	timer_interrupt_enable(TIMER4,TIMER_INT_UP);
	timer_enable(TIMER4);
	//timer_start(TIMER4,6911,TIMER_PSC_RELOAD_NOW);
}


