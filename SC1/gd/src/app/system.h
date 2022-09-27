#ifndef _SYSTEM_H_
#define _SYSTEM_H_
#include <stdint.h>
#define EEPROM_MEMORY_SIZE			(4096)
#define WATCHDOG_WDP				(0x07)

#define EEP_ADDR_EM4170_ID			(0xFF0)
#define EEP_ADDR_TAG63_ID			(0xFEE)

#define EM4170_ID_MAX_NUM			(6000)
#define TAG63_ID_MAX_NUM			(5000)

extern  const uint8_t a_tag64_uid1[3][6];
extern  const uint8_t a_tag63_uid1[5000][6];
extern  const uint8_t a_em4170_uid1[4000][8];
extern  const uint8_t a_em4170_uid2[EM4170_ID_MAX_NUM - 4000][8];
/*
void system_init(void);

#define watchdog_reset()			asm("WDR")

#define watchdog_off() \
		WDTCR = 0x18; \
		WDTCR = 0x00;
*/
// 改变定时器溢出时间及禁止(已经使能的)看门狗定时器需要执行一个特定的时间序列：
// 1. 在同一个指令内对WDCE 和WDE 写"1“，即使WDE 已经为"1“。
// 2. 在紧接的4 个时钟周期之内将WDE 和WDP 设置为合适的值，而WDCE 写"0”。
/*#define watchdog_on() \
		watchdog_reset(); \
		WDTCR = (0x18 | WATCHDOG_WDP); \
		WDTCR = (0x08 | WATCHDOG_WDP); \
		watchdog_reset();*/

//void interrupt_to_app(void);


uint8_t eep_rd_data(uint16_t addr, uint8_t *p_rd, uint8_t len);
uint8_t eep_wr_data(uint16_t addr, const uint8_t *p_wr, const uint8_t len);
void rf_ic_test(void);


#endif

