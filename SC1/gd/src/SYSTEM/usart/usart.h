#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

#define USART_REC_LEN  			224  	//定义最大接收字节数 200	  	
extern uint8_t  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern uint16_t USART_RX_STA;         		//接收状态标记	
extern void USART1_IRQHandler(void);
void uart_init(uint32_t uart,uint32_t bound);
void fputc_u2(char *f,uint8_t bytes);
#endif


