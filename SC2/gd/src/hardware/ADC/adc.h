#ifndef ADC_H
#define ADC_H
#include "headfile.h"
void adc_config(void);
void dma_config(uint32_t dma,dma_channel_enum dmach);
void adc_timer1trig_confg(void);
uint8_t timer_BATV_chek(uint32_t adc_periph,uint32_t external_trigger_source);
void dma_start(uint32_t dma,dma_channel_enum dmach,uint32_t periph_addr,uint32_t memadr,uint8_t sz);
#endif



