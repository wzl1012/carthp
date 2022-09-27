#ifndef _LFE3_H_
#define _LFE3_H_
#include "gd32f10x.h"
/********************************************************
 FPGA1_STATUS:				PL11A-->R22-->FPGA1_STATUS-->PF1			(creakOk--1)
 FPGA1_RXD:					PL8A<--R20<--FPGA1_RXD<--PF0				(default high level)
 FPGA1_TXD:					PL8B-->R21-->FPGA1_TXD-->PD0				(default high level)
********************************************************/
/*
#define LFE3_PW_CTL_DDRX			DDRA
#define LFE3_PW_CTL_PORTX			PORTA
#define LFE3_PW_CTL_PIN				4
		
#define LFE3_RESET_DDRX				DDRA
#define LFE3_RESET_PORTX			PORTA
#define LFE3_RESET_PIN				3
	
#define LFE3_TX_DDRX				DDRF
#define LFE3_TX_PORTX				PORTF
#define LFE3_TX_PIN					0

#define LFE3_RX_DDRX				DDRD
#define LFE3_RX_PORTX				PORTD
#define LFE3_RX_PINX				PIND
#define LFE3_RX_PIN					0

#define LFE3_STATUS_DDRX			DDRF
#define LFE3_STATUS_PORTX			PORTF
#define LFE3_STATUS_PINX			PINF
#define LFE3_STATUS_PIN				1

#define lfe3_power_output()			LFE3_PW_CTL_DDRX |= (1 << LFE3_PW_CTL_PIN)
#define lfe3_power_on()				LFE3_PW_CTL_PORTX |= (1 << LFE3_PW_CTL_PIN)
#define lfe3_power_off()			LFE3_PW_CTL_PORTX &= ~(1 << LFE3_PW_CTL_PIN)

#define lfe3_reset_output()			LFE3_RESET_DDRX |= (1 << LFE3_RESET_PIN)
#define lfe3_reset_high()			LFE3_RESET_PORTX |= (1 << LFE3_RESET_PIN)
#define lfe3_reset_low()			LFE3_RESET_PORTX &= ~(1 << LFE3_RESET_PIN)

#define lfe3_tx_output()			LFE3_TX_DDRX |= (1 << LFE3_TX_PIN)
#define lfe3_tx_high()				LFE3_TX_PORTX |= (1 << LFE3_TX_PIN)
#define lfe3_tx_low()				LFE3_TX_PORTX &= ~(1 << LFE3_TX_PIN)

#define lfe3_rx_input()				LFE3_RX_DDRX &= ~(1 << LFE3_RX_PIN)
#define lfe3_rx_pullup_dis()		LFE3_RX_PORTX &= ~(1 << LFE3_RX_PIN)
#define lfe3_rx_pullup_en()			LFE3_RX_PORTX |= (1 << LFE3_RX_PIN)
#define lfe3_rx_status				(LFE3_RX_PINX & (1 << LFE3_RX_PIN))

#define lfe3_sta_input()			LFE3_STATUS_DDRX &= ~(1 << LFE3_STATUS_PIN)
#define lfe3_sta_pullup_dis()		LFE3_STATUS_PORTX &= ~(1 << LFE3_STATUS_PIN)
#define lfe3_sta_pullup_en()		LFE3_STATUS_PORTX |= (1 << LFE3_STATUS_PIN)
#define lfe3_sta_status				(LFE3_STATUS_PINX & (1 << LFE3_STATUS_PIN))

#define LFE3_BAUD_9600_CYCLE				(104)
#define LFE3_BAUD_9600_HALF_CYCLE			(52)
*/
#define LFE3_RECE_TIMEOUT					(250U)
#define LFE3_CRACK_TIMEOUT					(180)

#define LFE3_SEND_START_LEN			(10)
#define LFE3_SEND_END_LEN			(5)
#define LFE3_SEND_4D_LEN			(17)
#define LFE3_RECE_4D_LEN			(9)
#define LFE3_SEND_46_LEN			(72)
#define LFE3_RECE_46_LEN			(20)

#define LFE3_TRY_TIMES				(2)
#define LFE3_INIT_SECOND			(10)

#define DISPLAY_CRACK_TIME			(1)


//void lfe3_init(void);

//void lfe3_send_data(const uint8_t *p_send, uint8_t bytes);
//uint8_t lfe3_rece_data(uint8_t *p_read, uint8_t bytes);
//void lfe3_stop_clock(void);
//void lfe3_start_clock(void);
//uint8_t lfe3_send_rece(const uint8_t *p_send, uint8_t send_len, uint8_t *p_read, uint8_t read_len, uint8_t rece_time);
//uint8_t lfe3_test_device(void);


#endif

