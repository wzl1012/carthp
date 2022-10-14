#ifndef _CMM_H
#define _CMM_H
#include "headfile.h"

#define  MIN_LV   13400//1.34*10000
#define  MAX_LV   18200//1.82*10000
#define  MDIFFS    100
#define  STEPV     (MAX_LV-MIN_LV)/MDIFFS
#define  FCMD     0


uint16_t checksum(uint8_t *u2rev,uint8_t datasz);
uint8_t v_to_per(float v_value);

#endif
