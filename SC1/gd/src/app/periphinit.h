#ifndef _PERIPHINIT_H_
#define _PERIPHINIT_H_
#include "headfile.h"


extern const unsigned char BT_DISCED_ICON_BMP[];
extern const unsigned char BT_CNED_ICON_BMP[];
//extern const unsigned char BAT_EMPT_ICON_BMP[];
//extern uint8_t temp[4];
extern  void rf_chk_init(void);
extern  void i2c_init(uint32_t i2cdev);
extern void delay_init(void);
extern void delay_us(uint32_t nus);
void periph_init(void)/*__attribute__ ((section(".ARM.__at_x08001000")))*/;



#endif
