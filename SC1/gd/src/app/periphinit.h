#ifndef _PERIPHINIT_H_
#define _PERIPHINIT_H_
#include "headfile.h"


extern const unsigned char BT_DISCED_ICON_BMP[];
extern const unsigned char BT_CNED_ICON_BMP[];
void BT_MOD_reset(void);
bool BT_u2sndr_cmd(char*cmd,char* rspd);

void rf_chk_init(void);
void i2c_init(uint32_t i2cdev);
void delay_init(void);
void delay_us(uint32_t nus);
void periph_init(void)/*__attribute__ ((section(".ARM.__at_x08001000")))*/;



#endif
