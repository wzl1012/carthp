#ifndef   _MAIN_H_
#define  _MAIN_H_
//#include <stdint.h>
//#include "semphr.h"
#include "headfile.h"
#define _INIT_TS_TAG		(1)

#if (_INIT_TS_TAG > 1)
#error   "check main function enable flag!"
#endif

#define T_DISPLAY			(2500)

#define __bit(x)			(1 << (x))


#define mem_size(array)		(sizeof(array) / sizeof(array[0]))
#define  FLSBASE     0x08000000
//#define  FLSOFF      0x16000
//extern SemaphoreHandle_t BSemaphore;
extern  TaskHandle_t CardrdTask_Handler;
extern  TaskHandle_t SLEEP_Task_Handler;
extern  TaskHandle_t POWER_M_Task_Handler;
extern  TaskHandle_t BTR_T_Task_Handler;

#endif
