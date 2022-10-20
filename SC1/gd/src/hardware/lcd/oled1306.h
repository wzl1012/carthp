#ifndef  OLED_1306_H
#define  OLED_1306_H
#include "headfile.h"
#define I2C0_SLAVE_ADDRESS7  0x78
#define OLED_DATA   0x40 
#define OLED_CMD  0x00
#define Max_Column	128
#define Max_Row		32



void oled1306_init(void);

#endif


