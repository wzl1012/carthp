#ifndef _TIMER_H_
#define _TIMER_H_
#include "gd32f10x.h"

//extern uint8_t t1_tick_16ms;
//extern uint8_t t1_tick_2s;
extern volatile uint8_t t2_tick_16ms;
extern volatile uint8_t t2_tick_1s;
#define  TIM1_PSC      (54000-1)      
#define  TIM1_PERID    (60000-1)   //50sX3 
#define timer1_stop()					    timer_disable(TIMER2)
//#define timer1_start_1()				(TCCR1B = ((TCCR1B&0xF8) | 0x01))
//#define timer3_start_1()				timer_start(TIMER1, 320)
//#define timer1_start_8()				(TCCR1B = ((TCCR1B&0xF8) | 0x02))
#define timer1_start_8()				timer_start(TIMER2,54,TIMER_PSC_RELOAD_NOW) //125us
//#define timer1_start_64()				(TCCR1B = ((TCCR1B&0xF8) | 0x03))
#define timer1_start_64()				timer_start(TIMER2,432,TIMER_PSC_RELOAD_NOW)//1000us
//#define timer3_start_256()				timer_start(TIMER1, 320)
//#define timer1_start_1024()				(TCCR1B = ((TCCR1B&0xF8) | 0x05))
//#define timer1_start_1024()			timer_start(TIMER1,108, TIMER_PSC_RELOAD_NOW)  //16ms
#define timer1_clear_ov()				timer_flag_clear(TIMER2,TIMER_FLAG_UP) 
#define timer1_ov_flag          timer_flag_get(TIMER2, TIMER_FLAG_UP)
#define TCNT1                   TIMER_CAR(TIMER2)
#define TCNT2                   TIMER_CAR(TIMER1)
#define TCNT2_V                  TIMER_CNT(TIMER1)
#define TCNT_V                  TIMER_CNT(TIMER2)
#define TCNT4_V                  TIMER_CNT(TIMER4)
#define timer2_ov_interrupt_en()		timer_interrupt_enable(TIMER2,TIMER_INT_UP)
#define timer2_ov_interrupt_dis()		timer_interrupt_disable(TIMER2,TIMER_INT_UP)
#define timer4_ov_interrupt_dis()		timer_interrupt_disable(TIMER4,TIMER_INT_UP)
#define timer2_stop()					   timer_disable(TIMER2)
#define timer4_stop()					   timer_disable(TIMER4)
//#define timer2_start_1024()				(TCCR2 = ((TCCR2&0xF8) | 0x05))
#define timer2_start_1024()				timer_start(TIMER2,6911, TIMER_PSC_RELOAD_NOW)  //16ms
void timer_start(uint32_t timer_periph,uint16_t Prescaler, uint32_t TIM_PSCReloadMode);
void user_timer_init(void);
void timer1_delay_us(uint16_t us);
void  timer4_start(void);

#endif

