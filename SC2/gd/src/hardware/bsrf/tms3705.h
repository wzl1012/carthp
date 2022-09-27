#ifndef _TMS3705_H_
#define _TMS3705_H_
#include "gpio.h"
/* TXCT : Control input from the microconroller(default value is high) */
#define tms3705_driver_off()			gpio_bit_set(GPIOC,  TMS3705OCT)
#define tms3705_driver_on()				gpio_bit_reset(GPIOC,  TMS3705OCT)

/* SCIO : Data output to the microcontroller */
#define scio_status						gpio_input_bit_get(GPIOC,TMS3705IN) 
void tms3705_init(void);         
uint8_t tms3705_rd_data(uint8_t *p_rd, uint8_t bytes);
uint8_t tms3705_init_charge(uint8_t mcr, uint16_t ms);
void tms3705_update_pwm_time(uint16_t t_off_l, uint16_t t_on_l, uint16_t t_off_h, uint16_t t_on_h);
void tms3705_pwm_wr_lsbit(const uint8_t *p_wr, uint8_t bits, uint8_t recharge_ms);
void tms3705_update_ppm_time(uint16_t t_off, uint16_t t_on_l, uint16_t t_on_h);
void tms3705_ppm_wr_lsbit(const uint8_t *p_wr, uint8_t bits, uint8_t recharge_ms);


#endif

