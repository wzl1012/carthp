#ifndef __KEY_H
#define __KEY_H	 
#include "gd32f10x.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//按键驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////   	 


//#define KEY0 PEin(4)   	//PE4
//#define KEY1 PEin(3)	//PE3 
//#define KEY2 PEin(2)	//PE2
//#define WK_UP PAin(0)	//PA0  WK_UP

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
#endif
