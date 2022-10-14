#ifndef _BT_H
#define _BT_H 
#include "headfile.h"
#define  BT_LKSTA  gpio_input_bit_get(GPIOC,GPIO_PIN_2)
#define   NOCMD  0x00
typedef enum{
 wrfail=0xe0,
 wrok=0xe1,
 rdfail=0xe2,
 sysfail=0xee		
}state_eum;

void BT_R_check(void);
void  BT_rdcmd_exe(uint8_t rdcmd);
bool  BT_send(uint8_t* pdata,uint8_t len,uint8_t pktype,uint8_t rdcmd);
bool u1_to_btsend(uint8_t cmd,uint8_t*pdata,uint8_t size);
uint8_t BT_recvdatadec(uint8_t *u2rev);
void BTR_T_task(void *pvParameters);
void u2_sendhex(uint8_t *f,uint8_t bytes);
#endif





















