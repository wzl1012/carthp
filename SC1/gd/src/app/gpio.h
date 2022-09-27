#ifndef GPIO_H
#define GPIO_H
#include "gd32f10x.h"
#define  CRDKYIN    GPIO_PIN_15
#define  PGDKYIN     GPIO_PIN_0
#define  RCCKYIN    GPIO_PIN_5
#define  TMS3705IN  GPIO_PIN_0
#define  TMS3705OCT GPIO_PIN_1

#define PCF7991_SCLK_OUT    GPIO_PIN_12
#define PCF7991DIN_OUT      GPIO_PIN_2
#define PCF7991DOUT_IN      GPIO_PIN_8
//#define keypad_read_level   gpio_input_bit_get(GPIOA,CRDKYIN)

//#define keypad_write_level  gpio_input_bit_get(GPIOC,RCCKYIN)
//#define   FC_RATE_IN     GPIO_PIN_6
//#define   FC_CODE_IN     GPIO_PIN_1
//lcd gpio define
#define  LCD_RD_PC6   GPIO_PIN_6
#define  LCD_WR_PC7   GPIO_PIN_7
#define  LCD_RS_PC8   GPIO_PIN_8
#define  LCD_CS_PC9   GPIO_PIN_9
#define  LCD_BL_PC7   GPIO_PIN_7
#define  PB0_7         0xFF
//#define  LCD_RESET    GPIO_PIN_8
#define   ICID_5V_CTRL_PB13   GPIO_PIN_13
#define   RFCHK_5V_CTRL_PB15   GPIO_PIN_15
#define   DEV_SEL_7991  gpio_bit_reset(GPIOC,GPIO_PIN_4)
#define   DEV_SEL_3705  gpio_bit_set(GPIOC,GPIO_PIN_4)
#define   DEV_SELPIN_level  gpio_output_bit_get(GPIOC,GPIO_PIN_4)
void user_kdl_init(void);
#endif
