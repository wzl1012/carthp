#include "sys.h"
#include "headfile.h"

#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"					//FreeRTOS使用	  
#endif
//////////////////////////////////////////////////////////////////////////////////	 


void fputc_u2(char *f,uint8_t bytes)
{  
   for(uint8_t i=0;i<bytes;i++){
		 	usart_data_transmit(USART1, (unsigned char) f[i]);
      while (!usart_flag_get(USART1, USART_FLAG_TBE)); 
	}		
}
int fputc(int ch, FILE *f)
{      
	usart_data_transmit(USART0, (unsigned char) ch);
  while (!usart_flag_get(USART0, USART_FLAG_TBE)); 
  return (ch);
}

 
void uart_init(uint32_t USARTx,uint32_t bound)
{
	 if(USARTx==USART0){
	    rcu_periph_clock_enable(RCU_USART0);
	 		usart_transmit_config(USARTx, USART_TRANSMIT_ENABLE);
	 }
  if(USARTx==USART1){
	    rcu_periph_clock_enable(RCU_USART1);
	}
	usart_baudrate_set(USARTx, bound);
	usart_parity_config(USARTx, USART_PM_NONE);
	usart_word_length_set(USARTx, USART_WL_8BIT);
	usart_stop_bit_set(USARTx, USART_STB_1BIT);
	usart_hardware_flow_rts_config(USARTx,USART_RTS_DISABLE);
	usart_hardware_flow_cts_config(USARTx,USART_CTS_DISABLE);
		//Usart2 NVIC
	  if(USARTx==USART1){
	    usart_interrupt_enable(USARTx, USART_INT_RBNE);
			usart_transmit_config(USARTx, USART_TRANSMIT_ENABLE);
	    usart_receive_config(USARTx, USART_RECEIVE_ENABLE);
		}
	   usart_enable(USARTx);                    //使能串口1 
}



