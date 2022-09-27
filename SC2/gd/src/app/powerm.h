#ifndef _POWERM_H_
#define _POWERM_H_
#include "headfile.h"
extern  uint16_t adc_value[1];
#define  keystart_sigal_level   gpio_input_bit_get(GPIOB,GPIO_PIN_14)
#define  pon_sigal_lock         gpio_bit_set(GPIOB,GPIO_PIN_12)
#define  poff_sigal_lock        gpio_bit_reset(GPIOB,GPIO_PIN_12)
#define  poctrl_sig_level       gpio_output_bit_get(GPIOB,GPIO_PIN_12)
#define  usbpow_sig_level       gpio_input_bit_get(GPIOB,GPIO_PIN_9)
#define  LCD_VCC_ON             gpio_bit_reset(GPIOC, LCD_BL_PC7)
#define  LCD_VCC_OFF            gpio_bit_set(GPIOC, LCD_BL_PC7)
#define  BT_VCC_ON           	  gpio_bit_reset(GPIOC, GPIO_PIN_9)
#define  BT_VCC_OFF           	gpio_bit_set(GPIOC, GPIO_PIN_9)
#define  ICD_5V_OFF             gpio_bit_reset(GPIOB,ICID_5V_CTRL_PB13)
#define  RFCHK_5V_OFF           gpio_bit_reset(GPIOB,RFCHK_5V_CTRL_PB15)
#define  ICD_5V_ON              gpio_bit_set(GPIOB,ICID_5V_CTRL_PB13)
#define  RFCHK_5V_ON            gpio_bit_set(GPIOB,RFCHK_5V_CTRL_PB15)
#define  ICD_5V_CTRL_PIN_LEVEL   gpio_output_bit_get(GPIOB,ICID_5V_CTRL_PB13)
#define  RFCHK_5V_CTRL_PIN_LEVEL   gpio_output_bit_get(GPIOB,RFCHK_5V_CTRL_PB15)

extern uint8_t volatile slpflg;
void power_mag(void);
void sleep_mag(void);
extern void fls_infs(void);
extern bool rf_chk(uint16_t* tmpint,uint16_t*tmpxs,uint8_t* fskflg);
#endif


