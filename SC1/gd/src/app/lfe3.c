#include "headfile.h"

const uint8_t str_test_fpga[LCD_ROW_LENGTH] =     "TestFPGA  V1.1  ";
const uint8_t str_fpga_starting[LCD_ROW_LENGTH] = "FPGA Starting.. ";
const uint8_t g_crack_4d_str[3][LCD_ROW_LENGTH] = {
	"4D Cracking1... ",
	"4D Cracking2... ",
	"4D Cracking3... "
};
const uint8_t g_crack_4d_str_result[2][LCD_ROW_LENGTH] = {
	"4D Crack OK     ",
	"4D Crack Fail   "
};
const uint8_t g_crack_46_str[3][LCD_ROW_LENGTH] = {
	"46 Cracking1... ",
	"46 Cracking2... ",
	"46 Cracking3... "
};
const uint8_t g_crack_46_str_result[2][LCD_ROW_LENGTH] = {
	"46 Crack OK     ",
	"46 Crack Fail   "
};

const uint8_t g_crack_start[LFE3_SEND_START_LEN] = {
	0xAA, 0xBB, 0xF1, 0xF1, 0x55, 0xAA, 0xBB, 0xF2, 0xF2, 0x55
};
const uint8_t g_crack_end[LFE3_SEND_END_LEN] = {
	0xAA, 0xBB, 0xF0, 0xF0, 0x55
};

const uint8_t g_crack_4d_send[3][LFE3_SEND_4D_LEN] = {
	0xAA, 0xBB, 0x4D, 0xA9, 0xBC, 0x72, 0x76, 0x73, 0x0A, 0xDF, 0xEE, 0x46, 0xFA, 0xC8, 0x3E, 0x5E, 0x55,
	0xAA, 0xBB, 0x4D, 0x62, 0xD0, 0xD5, 0x3B, 0xFD, 0xBE, 0x5E, 0x6B, 0x28, 0xBA, 0x69, 0x8B, 0x00, 0x55,
	0xAA, 0xBB, 0x4D, 0x0A, 0xB2, 0x03, 0x2F, 0x87, 0x88, 0xB0, 0x8D, 0xA1, 0x70, 0xC8, 0x93, 0x00, 0x55
};
const uint8_t g_crack_4d_rece[3][LFE3_RECE_4D_LEN] = {
	0xAA, 0xBB, 0x4D, 0xE8, 0x30, 0x68, 0x58, 0xCB, 0x55,
	0xAA, 0xBB, 0x4D, 0x68, 0x22, 0xA9, 0x73, 0xE5, 0x55,
	0xAA, 0xBB, 0x4D, 0xDA, 0x08, 0xAA, 0x5C, 0xF9, 0x55,
};

const uint8_t g_crack_46_send[3][LFE3_SEND_46_LEN] = {
	{0xAA, 0xBB, 0x46, 0x70, 0x8C, 0xCE, 0x34, 0x54, 0xE3, 0x8D, 0x38, 0x55, 
		0xAA, 0xBB, 0x46, 0xCB, 0xA7, 0xAB, 0x98, 0xB1, 0xFD, 0x07, 0x10, 0x55, 
		0xAA, 0xBB, 0x46, 0xFC, 0x06, 0x25, 0xD2, 0xEB, 0xDF, 0x77, 0x4A, 0x55, 
		0xAA, 0xBB, 0x46, 0xAC, 0x04, 0x84, 0xE7, 0x53, 0x77, 0xC3, 0x28, 0x55, 
		0xAA, 0xBB, 0x46, 0x68, 0x93, 0xC8, 0x97, 0xC6, 0x89, 0x31, 0xDE, 0x55, 
		0xAA, 0xBB, 0x46, 0xC5, 0x42, 0xF9, 0x66, 0x99, 0xC1, 0x85, 0xC1, 0x55},
	{0xAA, 0xBB, 0x46, 0x39, 0x2A, 0xE8, 0x07, 0xFA, 0xB0, 0xD0, 0x62, 0x55, 
		0xAA, 0xBB, 0x46, 0x1C, 0x15, 0xF4, 0x03, 0x7D, 0x58, 0x68, 0xB7, 0x55, 
		0xAA, 0xBB, 0x46, 0x8E, 0x0A, 0xFA, 0x81, 0x3E, 0x2C, 0x34, 0xDD, 0x55, 
		0xAA, 0xBB, 0x46, 0x47, 0x05, 0xFD, 0x40, 0x1F, 0x16, 0x9A, 0x68, 0x55, 
		0xAA, 0xBB, 0x46, 0xA3, 0x82, 0x7E, 0xA0, 0x0F, 0x0B, 0xCD, 0x32, 0x55, 
		0xAA, 0xBB, 0x46, 0x51, 0x41, 0x3F, 0xD0, 0x87, 0x85, 0x66, 0x9F, 0x55},
	{0xAA, 0xBB, 0x46, 0x3B, 0xB2, 0x98, 0x05, 0x55, 0xB9, 0x38, 0xC4, 0x55, 
		0xAA, 0xBB, 0x46, 0x1D, 0x59, 0xCC, 0x82, 0xAA, 0x5C, 0x1C, 0xE4, 0x55, 
		0xAA, 0xBB, 0x46, 0x8E, 0x2C, 0x66, 0x41, 0x55, 0x2E, 0x0E, 0xF4, 0x55, 
		0xAA, 0xBB, 0x46, 0x47, 0x16, 0xB3, 0xA0, 0x2A, 0x17, 0x87, 0xFC, 0x55, 
		0xAA, 0xBB, 0x46, 0x23, 0x8B, 0x59, 0x50, 0x95, 0x8B, 0xC3, 0x78, 0x55, 
		0xAA, 0xBB, 0x46, 0x91, 0xC5, 0x2C, 0xA8, 0xCA, 0xC5, 0xE1, 0x3A, 0x55}
};
const uint8_t g_crack_46_rece[3][LFE3_RECE_46_LEN] = {
	0xAA, 0xBB, 0x11, 0x46, 0xF2, 0x00, 0x70, 0x8C, 0xCE, 0x34, 0x54, 0xE3, 0x8D, 0x44, 0xE3, 0x96, 0x99, 0xC3, 0xF2, 0x55,
	0xAA, 0xBB, 0x11, 0x46, 0xF2, 0x00, 0x39, 0x2A, 0xE8, 0x07, 0xFA, 0xB0, 0xD0, 0x0E, 0xA3, 0x02, 0xD7, 0x91, 0xAA, 0x55,
	0xAA, 0xBB, 0x11, 0x46, 0xF2, 0x00, 0x3B, 0xB2, 0x98, 0x05, 0x55, 0xB9, 0x38, 0x6E, 0xF3, 0x30, 0xDB, 0xD7, 0x05, 0x55
};

const uint8_t g_crack_4d_rece_time[3] = {8, 16, 13};
const uint8_t g_crack_46_rece_time[3] = {20, 60, 130};
/*
void lfe3_init(void)
{
	lfe3_power_output();
	lfe3_power_on();				// R54 NC, always power on
	
	lfe3_tx_output();
	lfe3_tx_high();

	lfe3_rx_input();
	lfe3_rx_pullup_dis();			// if pullup enable, then rx is low, actual value is (Vcc / (Rmcu + Rfpga) * Rfpga), could no zero

	lfe3_sta_input();
	lfe3_sta_pullup_dis();

	lfe3_reset_output();
	lfe3_reset_high();
}
*/
void lfe3_send_data(const uint8_t *p_send, uint8_t bytes)
{
	uint8_t i, byte;

	while (bytes--) {
		byte = *p_send++;
		
		lfe3_tx_low();
		delay_us(LFE3_BAUD_9600_CYCLE);
		
		for (i = 0; i < 8; ++i) {
			if (byte & 0x01)
				lfe3_tx_high();
			else
				lfe3_tx_low();
			
			byte >>= 1;
			delay_us(LFE3_BAUD_9600_CYCLE - 1);
		}
		
		lfe3_tx_high();
		delay_us(LFE3_BAUD_9600_CYCLE);
		delay_us(LFE3_BAUD_9600_CYCLE);
	}
}

uint8_t lfe3_rece_data(uint8_t *p_read, uint8_t bytes)
{
#if DISPLAY_CRACK_TIME
#define FPGA_RECE_TIMEOUT		(2)
#else
#define FPGA_RECE_TIMEOUT		(50)
#endif

	uint8_t i, temp, first_byte;
	uint8_t delay_3s;

	first_byte = true;
	while (bytes--) {
		if (first_byte == true) {
			first_byte = false;
			for (delay_3s = 0; delay_3s < FPGA_RECE_TIMEOUT; delay_3s++) {
				timer1_stop();
				TCCR1B = 0x00;
				TCNT1 = (65536 - 60000);		// 64 * 60000 / 1000000 = 3.84 s
				timer1_clear_ov();
				timer1_start_1024();
				while ((lfe3_rx_status) && (timer1_ov_flag == 0));
				
				timer1_stop();
				timer1_clear_ov();
				if (lfe3_rx_status == 0)
					break;
			}
			if (delay_3s >= FPGA_RECE_TIMEOUT) {
				return false;				// 3.84 * FPGA_RECE_TIMEOUT
			}
		}
		else {
			timer1_stop();
			TCNT1 = (65536 - 60000);		// 0.5 * 60000 = 30 ms
			timer1_clear_ov();
			timer1_start_8();
			while ((lfe3_rx_status) && (timer1_ov_flag == 0));
			
			timer1_stop();
			timer1_clear_ov();
			if (lfe3_rx_status) {
				return false;				// 30 ms
			}
			delay_us(1);
		}

		/* detect start bit */
		delay_us(LFE3_BAUD_9600_HALF_CYCLE - 5);
		if (lfe3_rx_status) {
			return false;
		}
		delay_us(LFE3_BAUD_9600_HALF_CYCLE - 1);

		/* detect 8 bit read data */
		for (i = 0; i < 8; ++i) {
			delay_us(LFE3_BAUD_9600_HALF_CYCLE - 1);
			temp >>= 1;
			if (lfe3_rx_status)
				temp |= 0x80;
			else
				temp &= 0x7F;
			delay_us(LFE3_BAUD_9600_HALF_CYCLE - 1);
		}

		/* detect stop bit */
		delay_us(LFE3_BAUD_9600_HALF_CYCLE - 5);
		if (lfe3_rx_status == 0) {
			return false;
		}

		*p_read++ = temp;
	}

	return true;
}

#define LFE3_T_COMMU_GAP		(5)
void lfe3_stop_clock(void)
{
	lfe3_send_data(g_crack_end, LFE3_SEND_END_LEN);
	delay_1us(LFE3_T_COMMU_GAP*1000);
}

void lfe3_start_clock(void)
{
	lfe3_stop_clock();
	lfe3_send_data(&g_crack_start[0], 5);
	delay_1us(LFE3_T_COMMU_GAP*1000);
	lfe3_send_data(&g_crack_start[5], 5);
	delay_1us(LFE3_T_COMMU_GAP*1000);
}

uint8_t lfe3_send_rece(const uint8_t *p_send, uint8_t send_len, uint8_t *p_read, uint8_t read_len, const uint8_t rece_time)
{
	uint16_t i, time_out;
	uint8_t result;

	if (rece_time == 0)
		time_out = LFE3_CRACK_TIMEOUT;
	else
		time_out = rece_time;

	lfe3_start_clock();	
	lfe3_send_data(p_send, send_len);
	for (i = 0; i < (time_out * 10); i++) {
		delay_1us(100*1000);
		if (lfe3_sta_status) {
			delay_1us(100*1000);							// remove shake
			if (lfe3_sta_status)
				break;
		}
	}
	if (i < (time_out * 10))
		result = lfe3_rece_data(p_read, read_len);
	else
		result = false;
	
	lfe3_stop_clock();	

	return result;
}

uint8_t lfe3_test_device(void)
{
	uint8_t a_rece[21];
	uint8_t i, j, true_count;

	lcd_clear_screen();
	lcd_print_str(0, 0, str_test_fpga, LCD_ROW_LENGTH);
	lcd_print_str(0, 1, str_fpga_starting, LCD_ROW_LENGTH);

	delay_1us(10000*1000);
	true_count = 0;

	lcd_clear_screen();
	/* crack 4d */
	for (i = 0; i < LFE3_TRY_TIMES; i++) {
		lcd_print_str(0, 0, g_crack_4d_str[i], LCD_ROW_LENGTH);
		
		if (!lfe3_send_rece(g_crack_4d_send[i], LFE3_SEND_4D_LEN, a_rece, LFE3_RECE_4D_LEN, g_crack_4d_rece_time[i]))
			break;
		for (j = 0; j < LFE3_RECE_4D_LEN; j++)
			if (a_rece[j] != g_crack_4d_rece[i][j])
				break;
	}
	if ((i == LFE3_TRY_TIMES) && (j == LFE3_RECE_4D_LEN)) {
		true_count++;
		lcd_print_str(0, 0, g_crack_4d_str_result[0], LCD_ROW_LENGTH);
	}
	else
		lcd_print_str(0, 0, g_crack_4d_str_result[1], LCD_ROW_LENGTH);
	
	/* crack 46 */
	for (i = 0; i < LFE3_TRY_TIMES; i++) {
		lcd_print_str(0, 1, g_crack_46_str[i], LCD_ROW_LENGTH);
		
		if (!lfe3_send_rece(g_crack_46_send[i], LFE3_SEND_46_LEN, a_rece, LFE3_RECE_46_LEN, g_crack_46_rece_time[i]))
			break;
	}
	if (i == LFE3_TRY_TIMES) {
		true_count++;
		lcd_print_str(0, 1, g_crack_46_str_result[0], LCD_ROW_LENGTH);
	}
	else
		lcd_print_str(0, 1, g_crack_46_str_result[1], LCD_ROW_LENGTH);

	if (true_count == 2)
		return true;
	else
		return false;
}



