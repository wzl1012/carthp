#include "headfile.h"
uint16_t adc_value[1]={0};
void adc_config(void)
{
	  rcu_periph_clock_enable(RCU_ADC0);
    /* config ADC clock */
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV12);
   /* reset ADC */
    adc_deinit(ADC0);
    /* ADC mode config */
    adc_mode_config(ADC_MODE_FREE);
    /* ADC scan function enable */
    //adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, ENABLE);
	   /* ADC scan mode disable */
   // adc_special_function_config(ADC0, ADC_SCAN_MODE, DISABLE);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);

    /* ADC channel length config */
    adc_channel_length_config(ADC0,ADC_REGULAR_CHANNEL, 1);

    /* ADC temperature sensor channel config */
    adc_regular_channel_config(ADC0,0, ADC_CHANNEL_1, ADC_SAMPLETIME_239POINT5);
    /* ADC trigger config */
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);
 
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);
	  adc_flag_clear(ADC0,ADC_FLAG_EOC);
    /* enable ADC interface */
    adc_enable(ADC0);
    delay_us(1000);
    /* ADC calibration and reset calibration */
    adc_calibration_enable(ADC0);
		        /* ADC DMA function enable */
    adc_dma_mode_enable(ADC0);
}

void dma_config(uint32_t dma,dma_channel_enum dmach)	
{
	  if(dma==DMA0)
	  rcu_periph_clock_enable(RCU_DMA0);
		else
		rcu_periph_clock_enable(RCU_DMA1);	
    /* ADC_DMA_channel configuration */
    dma_parameter_struct dma_data_parameter={0};
    
    /* ADC DMA_channel configuration */
    dma_deinit(dma, dmach);
    if(dmach==DMA_CH0){
    /* initialize DMA single data mode */
    dma_data_parameter.periph_addr  =(uint32_t)(&ADC_RDATA(ADC0));
    dma_data_parameter.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_data_parameter.memory_addr  = (uint32_t)(&adc_value);
    dma_data_parameter.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_data_parameter.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_data_parameter.memory_width = DMA_MEMORY_WIDTH_16BIT;  	
    dma_data_parameter.direction    = DMA_PERIPHERAL_TO_MEMORY;
    dma_data_parameter.number       = 1;
    dma_data_parameter.priority     = DMA_PRIORITY_HIGH;
	}
	if(dmach==DMA_CH1){
	  //dma_data_parameter.periph_addr  =FLS_SEC2_DLADR;
		dma_data_parameter.periph_inc   = DMA_PERIPH_INCREASE_ENABLE;	
    //dma_data_parameter.memory_addr  = memadr/*(uint32_t)(&adc_value)*/;
    dma_data_parameter.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_data_parameter.periph_width = DMA_PERIPHERAL_WIDTH_32BIT;
    dma_data_parameter.memory_width = DMA_MEMORY_WIDTH_32BIT;  
    dma_data_parameter.direction    = DMA_MEMORY_TO_PERIPHERAL;
   // dma_data_parameter.number       = trsz;
    dma_data_parameter.priority     = DMA_PRIORITY_ULTRA_HIGH;
		DMA_CHCTL(dma, dmach)|=DMA_CHXCTL_M2M;
		}
    dma_init(dma, dmach, &dma_data_parameter);
    /* enable DMA circulation mode */
		if(dmach==DMA_CH0){
    dma_circulation_enable(dma, dmach);
  
    /* enable DMA channel */
    dma_channel_enable(dma,dmach);
		}
}

void dma_start(uint32_t dma,dma_channel_enum dmach,uint32_t periph_addr,uint32_t memadr,uint8_t sz)	
{
    /* disable DMA circulation mode */
    //dma_circulation_disable(dma, dmach);
  
    /* enable DMA channel */
    dma_channel_disable(dma,dmach);
	  dma_flag_clear(dma,dmach,DMA_FLAG_FTF);
	   DMA_CHPADDR(dma, dmach) = periph_addr;
	  DMA_CHMADDR(dma, dmach) = memadr;
	  DMA_CHCNT(dma, dmach)=(sz& DMA_CHANNEL_CNT_MASK);
	  //dma_circulation_enable(dma, dmach);
	  dma_channel_enable(dma,dmach);
}
void adc_timer1trig_confg(void)
{
			    /* ADC trigger config */
		adc_disable(ADC0);
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_EXTTRIG_REGULAR_T1_CH1);
		adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);
		adc_flag_clear(ADC0,ADC_FLAG_EOC);
	  adc_value[0]=0;
		adc_enable(ADC0);
		TCNT2_V =0;
		timer_enable(TIMER1);	
}

uint8_t timer_BATV_chek(uint32_t adc_periph,uint32_t external_trigger_source)
{
	//if(adc_flag_get(ADC0,ADC_FLAG_EOC)==SET){
	//	adc_flag_clear(ADC0,ADC_FLAG_EOC);
		 if(adc_value[0]){
			 float tmpv=0;
       tmpv=(adc_value[0] * 3.3 / 4096);
			 adc_value[0]=0;
			 return v_to_per(tmpv);
		 }		
	//}
	return 0xFF;  
}

