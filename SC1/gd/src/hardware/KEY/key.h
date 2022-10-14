#ifndef __KEY_H
#define __KEY_H	 
#include "gd32f10x.h"


#define keypad_read_level   gpio_input_bit_get(GPIOA,GPIO_PIN_15)//读取按键0
#define keypad_write_level  gpio_input_bit_get(GPIOC,GPIO_PIN_5)//读取按键1
#define keypad_FCHK_level  gpio_input_bit_get(GPIOA,GPIO_PIN_0)//读取按键2 
//#define keypad_write_level GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//读取按键3(WK_UP) 
 
typedef enum
{
	not_find=0,
	d7991_find=1,
	d3705_find,
} dev_findrslt_t;

#define KEYPAD_RD_PRES 	1	//KEY0按下
#define KEYPAD_WR_PRES	2	//KEY1按下
#define KEYPAD_FCHK_PRES	3	//KEY2按下
#define KEYPAD_NO_PRES	0	//KEY2按下
#define KEYSTART_SIG_PRES 4
//#define WKUP_PRES   4	//KEY_UP按下(即WK_UP/KEY_UP)
extern volatile uint8_t rfckstart;
uint8_t KEY_Scan(uint8_t);  	//按键扫描函数	
void keypad_event_check(void);
void Key_EXTI_Init(void);
void cardrd_task(void *pvParameters);
#endif
