#ifndef _PCF7991_H_
#define _PCF7991_H_
#include "gd32f10x.h"
#define pcf7991_sclk_high()			gpio_bit_set(GPIOC,  PCF7991_SCLK_OUT)
#define pcf7991_sclk_low()			gpio_bit_reset(GPIOC,  PCF7991_SCLK_OUT)

#define pcf7991_driver_off()		gpio_bit_set(GPIOD,  PCF7991DIN_OUT)
#define pcf7991_driver_on()			gpio_bit_reset(GPIOD,  PCF7991DIN_OUT)


#define pcf7991_dout_level			gpio_input_bit_get(GPIOA,PCF7991DOUT_IN)


#define PCF7991_GET_SAMPLING_TIME				(0x02)
#define PCF7991_GET_CONFIG_PAGE					(0x04)
#define PCF7991_READ_PHASE						(0x08)
#define PCF7991_READ_TAG						(0xE0)
#define PCF7991_WRITE_TAG_N						(0x10)
#define PCF7991_WRITE_TAG						(0xC0)
#define PCF7991_SET_CONFIG_PAGE					(0x40)
#define PCF7991_SET_SAMPLE_TIME					(0x80)

#define TS06_PULSE_NUM							(0x07)
#define SIC7888_PULSE_NUM						(0x07)
 
/*after device power-up, sampling time shift or when switching from WRITE_TAG mode to READ_TAG mode.
the bandpass filter, amplifier and digitizer circuit biasing condition can be initialized 
and restored by a set of control bits accessible via the SET_CONFIG_PAGE commands.   */
#define PCF7991_PAGE0_NORMAL					(0x0F)
#define PCF7991_PAGE0_PCF7936_NEW_BOARD			(0x0F)	//fit TS888 new PCB
#define PCF7991_PAGE0_PCF7936_OLD_BOARD			(0x0B)	//fit TS888 old PCB
#define PCF7991_PAGE0_TS06						(0x07)
#define PCF7991_PAGE0_SIC7888					(0x0B)
#define PCF7991_PAGE0_AT5577					(0x0B)
#define PCF7991_PAGE0_PCF7935					(0x07)
#define PCF7991_PAGE0_EM4170					(0x0F)
#define PCF7991_PAGE0_EM4100					(0x0F)	// 0x0F better then  0x0B
#define PCF7991_PAGE0_NOVA						(0x0F)	// 0x0F better then  0x0B
#define PCF7991_PAGE0_E5561						(0x0F)

#define pcf7991_active_driver()				pcf7991_set_page(0x01, 0x02)
#define pcf7991_inactive_driver()			pcf7991_set_page(0x01, 0x03)
#define DECODE_MAX_T						(3000 * 2)
typedef enum
{
	e_modu_mc = 0x10,
	e_modu_bp
} modulation_t;
typedef enum
{
	e_dir_msbit = 0x20,
	e_dir_lsbit
} byte_dir_t;
typedef enum
{
	page0=0,
	page1,
	page2,
	page3,
} pg_t;
void pcf7991_read_tag(void);
void pcf7991_write_tag_n(uint8_t n);
void pcf7991_write_tag(void);
void pcf7991_set_sample_time(void);
void pcf7991_set_page(uint8_t page, uint8_t data);
void pcf7991_exit_tag_mode(void);
uint8_t pcf7991_read_phase(void);
uint8_t pcf7991_get_sampling_time(void);
uint8_t pcf7991_get_config_page(uint8_t page);
void pcf7991_init(void);
void pcf7991_confg(void);
void pcf7991_tagrd_gsetproc(void);
void pcf7991_tagrd_fsetproc(void);
bool pcf7991_turn_dout(uint8_t ms_charge, uint8_t page0, uint8_t find_ms, uint8_t times);
bool pcf7991_decode_miss_low(void);
//bool pcf7991_48tagdecode_miss_low(void);
bool pcf7991_decode_mc(const uint8_t bit_period, uint8_t *p_rd, const uint8_t bits, uint8_t dir);
bool pcf7991_decode_mc2(const uint8_t bit_period, uint8_t *p_rd, const uint8_t bits, uint8_t dir);
bool pcf7991_decode_bp(const uint8_t bit_period, uint8_t *p_rd, const uint8_t bits, uint8_t dir);
bool pcf7991_decode_anti_c(const uint8_t bit_period, uint8_t *p_rd, const uint8_t bits, uint8_t dir);


#endif

