#include "headfile.h"

#define TMS3705_T_R					(15000)			// Delay between end of charge or end of program and start of transponder data transmit on SCIO
#define TMS3705_T_SCI				(64)			// NRZ bit duration on SCIO
#define TMS3705_T_INIT				(6)				// Time for TXCT high to initialize a new transmission
#define TMS3705_T_MCR				(128)			// NRZ bit duration for mode control register
#define TMS3705_T_DIAG				(3)				// Delay between leaving idle mode and start of diagnosis byte at SCIO

typedef struct
{
	uint16_t t_off_l;
	uint16_t t_on_l;
	uint16_t t_off_h;
	uint16_t t_on_h;
} tms3705_pwm_time_t;

typedef struct
{
	uint16_t t_off;
	uint16_t t_on_l;
	uint16_t t_on_h;
} tms3705_ppm_time_t;

static tms3705_pwm_time_t pwm_time;
static tms3705_ppm_time_t ppm_time;
/*void tms3705_init(void)
{
	tms3705_driver_on();
	delay_us(300*1000);

	tms3705_driver_off();
	delay_us(600*1000);
}*/
void tms3705_init(void)
{
	tms3705_driver_on();
	delay_us(400);
	tms3705_driver_off();
	//delay_us(600000);
}
void tms3705_update_pwm_time(uint16_t t_off_l, uint16_t t_on_l, uint16_t t_off_h, uint16_t t_on_h)
{
	pwm_time.t_off_l = t_off_l;
	pwm_time.t_on_l = t_on_l;
	pwm_time.t_off_h = t_off_h;
	pwm_time.t_on_h = t_on_h;
}

void tms3705_pwm_wr_lsbit(const uint8_t *p_wr, uint8_t bits, uint8_t recharge_ms)
{
	uint8_t i, byte=0;
	for (i = 0; i < bits; i++) {
		if ((i % 8) == 0)
			byte = *p_wr++;

		tms3705_driver_off();
		timer1_stop();
		TCNT1 = (byte & 0x01) ? (/*65536 - */(pwm_time.t_off_h<<1)-3)/* + 3)*/ 
							: (/*65536 -*/ (pwm_time.t_off_l<<1)-3)/* + 3)*/;	// +3 ensure accurate time
		timer1_clear_ov();
		timer1_start_8();
		while (timer1_ov_flag == 0);
		
		tms3705_driver_on();
		timer1_stop();
		TCNT1 = (byte & 0x01) ? (/*65536 -*/ (pwm_time.t_on_h<<1)-3)/* + 3)*/ 
							: (/*65536 -*/ (pwm_time.t_on_l<<1)-3)/* + 3)*/;
		timer1_clear_ov();
		timer1_start_8();
		while (timer1_ov_flag == 0);
		
		timer1_stop();
		timer1_clear_ov();
		
		byte >>= 1;
	}

	delay_us((uint16_t)(recharge_ms*1000));
	tms3705_driver_off();
}

void tms3705_update_ppm_time(uint16_t t_off, uint16_t t_on_l, uint16_t t_on_h)
{
	ppm_time.t_off = t_off;
	ppm_time.t_on_l = t_on_l;
	ppm_time.t_on_h = t_on_h;
}

void tms3705_ppm_wr_lsbit(const uint8_t *p_wr, uint8_t bits, uint8_t recharge_ms)
{
	uint8_t byte=0;
	uint8_t i = 0;
	for (i = 0; i < bits; i++) {
		if ((i % 8) == 0)
			byte = *p_wr++;
		tms3705_driver_off();
		timer1_stop();
		TCNT1 = (/*65536 -*/ (ppm_time.t_off<<1)-2)/* + 2)*/;	// +2 ensure accurate time
			timer1_clear_ov();
		timer1_start_8();
		while (timer1_ov_flag == 0);

		tms3705_driver_on();
		timer1_stop();
		TCNT1 = (byte & 0x01) ? (/*65536 -*/ (ppm_time.t_on_h<<1)-2)/* + 2) */
							: (/*65536 -*/ (ppm_time.t_on_l<<1)-2)/* + 2)*/;
		timer1_clear_ov();
		timer1_start_8();
		while (timer1_ov_flag == 0);

		timer1_stop();
		timer1_clear_ov();
		
		byte >>= 1;
	}

	if (recharge_ms > 0) {
		tms3705_driver_off();
		timer1_stop();
		TCNT1 = (/*65536 -*/ (ppm_time.t_off<<1)-2)/* + 2)*/;
		timer1_clear_ov();
		timer1_start_8();
		while (timer1_ov_flag == 0);

		timer1_stop();
		timer1_clear_ov();
		tms3705_driver_on();
		delay_us((uint16_t)(recharge_ms*1000));
	}
	tms3705_driver_off();
}

/********************************************************
* function: TMS3705 return data, MCU decode and read
*
* 15.625k baud(64us/bit)
* one start bit(high)
* one stop bit(low)
* LSBit-->MSBit
*
* p_rd: pointer for store read data
* bytes: bytes of read data
*
* return: if read bytes success, return TRUE
********************************************************/
uint8_t tms3705_rd_data(uint8_t *p_rd, uint8_t bytes)
{
	uint8_t i, middle_flag, byte=0;

	while (bytes--) {
		timer1_stop();
		TCNT1 = (/*65536 -*/ TMS3705_T_R * 2);
		timer1_clear_ov();
		timer1_start_8();
		while ((scio_status == 0) && (timer1_ov_flag == 0));

		timer1_stop();
		if (timer1_ov_flag) {
			timer1_clear_ov();
			return FALSE;				// no find 
		}

		/* detect start bit */
		TCNT1 = (/*65536 - */TMS3705_T_SCI*2-3)/* + 3)*/;	// +3 could ensure accurate time
		timer1_start_8();
		while (timer1_ov_flag == 0) {
			if ((TCNT_V >= (/*65536 - */TMS3705_T_SCI - 5)) 
					&& (TCNT_V <= (/*65536 -*/ TMS3705_T_SCI + 5)) 
					&& (scio_status == 0)) {
				timer1_stop();
				return FALSE;			// start bit error
			}
		}

		/* detect 8 bit read data */
		for (i = 0; i < 8; ++i) {
			timer1_stop();
			TCNT1 = (/*65536 - */TMS3705_T_SCI*2-3)/* + 3)*/;	// +3 could ensure accurate time
			timer1_clear_ov();
			timer1_start_8();
			middle_flag = TRUE;
			while (timer1_ov_flag == 0) {
				if ((TCNT_V >= (/*65536 - */TMS3705_T_SCI - 5))
						&& (TCNT_V <= (/*65536 -*/ TMS3705_T_SCI + 5)) 
						&& (middle_flag == TRUE)) {
					middle_flag = FALSE;
					byte >>= 1;
					/* The data bits at the SCIO output are inverted with respect 
					to the corresponding bits sent by the transponder. */
					if (scio_status)
						byte &= (~0x80);
					else
						byte |= 0x80;
				}
			}
		}

		/* detect stop bit */
		timer1_stop();
		timer1_clear_ov();
		TCNT1 = (/*65536 -*/ TMS3705_T_SCI*2-4)/* + 4)*/;	// +4 could ensure accurate time
		timer1_start_8();
		while (timer1_ov_flag == 0) {
			if ((TCNT_V >= (/*65536 -*/ TMS3705_T_SCI - 5)) 
					&& (TCNT_V <= (/*65536 -*/ TMS3705_T_SCI + 5)) 
					&& (scio_status == 1)) {
				timer1_stop();
				return FALSE;		// stop bit error
			}
		}
		timer1_stop();
		timer1_clear_ov();

		*p_rd++ = byte;
	}

	return TRUE;
}

static uint8_t tms3705_modecontrol_write(uint8_t mode)
{
	uint8_t i;

	if (mode & 0x01)
		return FALSE;

	for (i = 0; i < 8; i++) {
		if (mode & 0x01)
			tms3705_driver_off();
		else
			tms3705_driver_on();
		delay_us(TMS3705_T_MCR - 1);

		mode >>= 1;
	}

	return TRUE;
}

/************************************************
* mcr typical value is 0x00
* ms typical value is 50
************************************************/
#if _INIT_TS_TAG
#define TAG_T_RESET_MAX			(50)
#else
#define TAG_T_RESET_MAX			(80)
#endif
uint8_t tms3705_init_charge(uint8_t mcr, uint16_t ms)
{
	uint8_t result, diag_byte;

	result = FALSE;

	tms3705_driver_off();
	delay_us(TAG_T_RESET_MAX*1000);
	delay_us(TMS3705_T_INIT*1000);
	
	tms3705_driver_on();
	if (tms3705_modecontrol_write(mcr))
		if (tms3705_rd_data(&diag_byte, 1))
			if (diag_byte == 0x50)			// In case of a normal operation of the antenna, the diagnostic byte AF_H is sent.
				result = TRUE;

	if (result) {
		tms3705_driver_on();
		delay_us(ms*1000);
	}
	else
		tms3705_driver_off();

	return result;
}


