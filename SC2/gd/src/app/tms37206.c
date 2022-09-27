// When the protection bit is set, programming to the corresponding memory Page is only possible with mutual authentication programming. 


#include "headfile.h"

tms37206_tag_t tms37206_tag1;

#define tms37206_T_CHARGE					(50)
#define tms37206_T_RESET					(15)
#define tms37206_T_PROG						(15 + 5)
#define tms37206_T_ENC						(4 + 4)
#define tms37206_T_MUT						(8 + 8)
#define tms37206_T_PWM_READ_RECH			        (0 + 8)

#define tms37206_T_HDET_6					(356)
#define tms37206_T_HDET_8					(476)
#if 1
/* tms37206 datasheet narrow spread time */
#define tms37206_PWM_T_OFF_L				        (170)
#define tms37206_PWM_T_ON_L					(240)
#define tms37206_PWM_T_OFF_H				        (170)
#define tms37206_PWM_T_ON_H					(360)
#define tms37206_SOF_T_S_L					(170)
#define tms37206_SOF_T_S_H					(480)
#define tms37206_SOF_T_0_L					(170)
#define tms37206_SOF_T_0_H					(240)
#define tms37206_SOF_T_1_L					(170)
#define tms37206_SOF_T_1_H					(360)
#define tms37206_EOF_T_L					(170)
#define tms37206_EOF_T_H					(570)

#else
/* tms37206 datasheet wide spread time */
#define tms37206_PWM_T_OFF_L				        (120)
#define tms37206_PWM_T_ON_L					(230)
#define tms37206_PWM_T_OFF_H				        (120)
#define tms37206_PWM_T_ON_H					(460)
#define tms37206_SOF_T_S_L					(120)
#define tms37206_SOF_T_S_H					(690)
#define tms37206_SOF_T_0_L					(120)
#define tms37206_SOF_T_0_H					(230)
#define tms37206_SOF_T_1_L					(120)
#define tms37206_SOF_T_1_H					(460)
#define tms37206_EOF_T_L					(120)
#define tms37206_EOF_T_H					(920)
#endif

uint16_t tms37206_calc_crc16(const uint8_t *p_data, uint8_t bytes)
{
	uint8_t i;
	uint16_t crc;

	crc = 0x3791;				// ????
	while (bytes--) {
		crc ^= *p_data++;
		for (i = 0; i < 8; i++) {
			if (crc & 0x0001) {
				crc >>= 1;
				crc ^= 0x8408;
			}
			else
				crc >>= 1;
		}
	}

	return crc;
}


void tm37206_sign_wr()
{
        tms3705_driver_off();       // S
        delay_us(tms37206_SOF_T_S_L);
        tms3705_driver_on();
        delay_us(tms37206_SOF_T_S_H);
        tms3705_driver_off();       // 0
        delay_us(tms37206_SOF_T_0_L);
        tms3705_driver_on();
        delay_us(tms37206_SOF_T_0_H);
        tms3705_driver_off();       // 1
        delay_us(tms37206_SOF_T_1_L);
        tms3705_driver_on();
        delay_us(tms37206_SOF_T_1_H); 
        
}

void tms37206_eof()
{
        tms3705_driver_off();
        delay_us(tms37206_EOF_T_L);
        tms3705_driver_on();       // EOF
        delay_us(tms37206_EOF_T_H);
        tms3705_driver_off();
}
//*********************************************************************//
//37206发送一个BIT的数据
//
//*********************************************************************//
void tms37206_pwm_wr_lsbit(const uint8_t *p_wr, uint8_t bits, uint8_t recharge_ms)
{
	uint8_t i, byte=0;
               
	for (i = 0; i < bits; i++) 
	{
		if ((i % 8) == 0)
			byte = *p_wr++;

		tms3705_driver_off();
		if (byte & 0x01)
			delay_us(tms37206_PWM_T_OFF_H - 1);
		else
			delay_us(tms37206_PWM_T_OFF_L - 1);
		
		tms3705_driver_on();
		if (byte & 0x01)
			delay_us(tms37206_PWM_T_ON_H - 1);
		else
			delay_us(tms37206_PWM_T_ON_L - 1);
		
		byte >>= 1;
	}
       
	delay_us((uint16_t)recharge_ms*1000);
	tms3705_driver_off();
}
//*********************************************************************//
//模式：PWM模式
//发送命令+低位在前+读取返回的数据
//
//*********************************************************************//
uint8_t tms37206_pwm_wr_ls_rd(uint8_t bytes_wr, const uint8_t *p_wr,uint8_t recharge_ms, uint8_t *p_rd)
{
	uint8_t result;
	uint8_t a_read[14];

	result = FALSE;
       
	if (tms3705_init_charge(0x00, tms37206_T_CHARGE)) {
                tm37206_sign_wr();  // send S 0 1
		tms37206_pwm_wr_lsbit(p_wr, bytes_wr*8, 0);
                tms37206_eof();     // eof
                
                
                tms3705_driver_on();
                delay_us((uint16_t)recharge_ms*1000);
	        tms3705_driver_off();                
		if (tms3705_rd_data(a_read, 14)) {
			copy_bytes(p_rd, a_read, 14);
			result =   TRUE;
		}
	}
        delay_us(tms37206_T_RESET*1000);
	return result;
}
//*********************************************************************//
//模式：PWM模式
//发送命令+低位在前+CRC校验+读取返回的数据
//
//*********************************************************************//
uint8_t tms37206_pwm_wr_ls_crc_rd(uint8_t bytes_wr, const uint8_t *p_wr, uint8_t recharge_ms ,uint8_t *p_rd)
{
	uint8_t a_crc[2];
	uint16_t crc;
	uint8_t result;
	uint8_t a_read[14];
	crc = tms37206_calc_crc16(p_wr, bytes_wr);
	a_crc[0] = (uint8_t)crc;
	a_crc[1] = (uint8_t)(crc>>8);


	result = FALSE;
	if (tms3705_init_charge(0, tms37206_T_CHARGE)) {
                tm37206_sign_wr();  // send S 0 1
		tms37206_pwm_wr_lsbit(p_wr, bytes_wr*8, 0);
		tms37206_pwm_wr_lsbit(a_crc, 16, 0);
                tms37206_eof();     // eof      
               
                tms3705_driver_on();
                delay_us((uint16_t)recharge_ms*1000);
	        tms3705_driver_off();
               
		if (tms3705_rd_data(a_read, 14)) {
	             copy_bytes(p_rd, a_read, 14);
			result =   TRUE;
		}
	}
        delay_us(tms37206_T_RESET*1000);
	return result;
}

void tms37206_fast_read_init(void)
{
	LCD_Clear(BLACK);
	lcd_print_str(0, 0, "FastRead H V1.0", 15);

	//led_on();
	//delay_us(500);
	//led_off();
}

uint8_t tms37206_fast_read(void)
{
	uint8_t result,i_fail=0,a_rd[12];
	uint8_t a_wr_0[2] = {0xf0,0x00};
        uint8_t a_wr_1[2] = {0xf0,0x01};
        uint8_t a_wr_2[2] = {0xf0,0x02};
        uint8_t a_wr_3[2] = {0xf0,0x03};
        uint8_t a_wr_4[2] = {0xf0,0x04};
        uint8_t a_wr_5[2] = {0xf0,0x05};
        uint8_t a_wr_6[2] = {0xf0,0x06};
        uint8_t a_wr_7[2] = {0xf0,0x07};
        uint8_t a_wr_8[2] = {0xf0,0x08};
        uint8_t a_wr_9[2] = {0xf0,0x09};
        uint8_t a_wr_10[2] = {0xf0,0x0a};
        uint8_t a_wr_11[2] = {0xf0,0x0b};
        uint8_t a_wr_12[2] = {0xf0,0x0c};
        uint8_t a_wr_13[2] = {0xf0,0x0d};
        uint8_t a_wr_14[2] = {0xf0,0x0e};
        uint8_t a_wr_15[2] = {0xf0,0x0f};
	uint8_t a_wr_16[2] = {0xf0,0x10};
        uint8_t a_wr_17[2] = {0xf0,0x11};
        uint8_t a_wr_18[2] = {0xf0,0x12};
        uint8_t a_wr_19[2] = {0xf0,0x13};
        uint8_t a_wr_20[2] = {0xf0,0x14};
        uint8_t a_wr_21[2] = {0xf0,0x15};
        uint8_t a_wr_22[2] = {0xf0,0x16};
        uint8_t a_wr_23[2] = {0xf0,0x17};
        uint8_t a_wr_24[2] = {0xf0,0x18};
        uint8_t a_wr_25[2] = {0xf0,0x19};
        uint8_t a_wr_26[2] = {0xf0,0x1a};
        uint8_t a_wr_27[2] = {0xf0,0x1b};
        uint8_t a_wr_28[2] = {0xf0,0x1c};
        uint8_t a_wr_29[2] = {0xf0,0x1d};
        uint8_t a_wr_30[2] = {0xf0,0x1e};
        uint8_t a_wr_31[2] = {0xf0,0x1f};        

	enum {
		s_get_data_0,s_get_data_1,
                s_get_data_2,s_get_data_3,
                s_get_data_4,s_get_data_5,
                s_get_data_6,s_get_data_7,
                s_get_data_8,s_get_data_9,
                s_get_data_10,s_get_data_11,
                s_get_data_12,s_get_data_13,
                s_get_data_14,s_get_data_15,
                s_get_data_16,s_get_data_17,
                s_get_data_18,s_get_data_19,
                s_get_data_20,s_get_data_21,
                s_get_data_22,s_get_data_23,
                s_get_data_24,s_get_data_25,
                s_get_data_26,s_get_data_27,
                s_get_data_28,s_get_data_29,
                s_get_data_30,s_get_data_31,
		s_ok, s_wait_out, s_exit
	} state;

	result = FALSE;
	state = s_get_data_0;
	while (state != s_exit) {
		switch (state) {
			case s_get_data_0:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_0[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                  //      uart_send_str(&a_rd[2], 4);
					state = s_get_data_1;
                                }
				break;
                        case s_get_data_1:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_1[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                   //     uart_send_str(&a_rd[2], 4);
					state = s_get_data_2;
                                }
				break;
                        case s_get_data_2:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_2[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                  //      uart_send_str(&a_rd[2], 4);
					state = s_get_data_3;
                                }
				break;
                       case s_get_data_3:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_3[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                       // uart_send_str(&a_rd[2], 4);
					state = s_get_data_4;
                                }
				break;  
                       case s_get_data_4:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_4[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                      //  uart_send_str(&a_rd[2], 4);
					state = s_get_data_5;
                                }
				break;         
                       case s_get_data_5:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_5[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                      //  uart_send_str(&a_rd[2], 4);
					state = s_get_data_6;
                                }
				break;         
                       case s_get_data_6:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_6[0], 0,a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                       // uart_send_str(&a_rd[2], 4);
					state = s_get_data_7;
                                }
				break;         
                                
                       case s_get_data_7:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_7[0], 0,a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                      //  uart_send_str(&a_rd[2], 4);
					state = s_get_data_8;
                                }
				break;         
                       case s_get_data_8:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_8[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                      //  uart_send_str(&a_rd[2], 4);
					state = s_get_data_9;
                                }
				break;         
                       case s_get_data_9:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_9[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                        //uart_send_str(&a_rd[2], 4);
					state = s_get_data_10;
				break;   
                       case s_get_data_10:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_10[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                       // uart_send_str(&a_rd[2], 4);
					state = s_get_data_11;
                                }
				break;         
                       case s_get_data_11:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_11[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                      //  uart_send_str(&a_rd[2], 4);
					state = s_get_data_12;
                                }
				break;         
                       case s_get_data_12:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_12[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                      //  uart_send_str(&a_rd[2], 4);
					state = s_get_data_13;
                                }
				break;         
                      case s_get_data_13:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_13[0], 0,a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                       // uart_send_str(&a_rd[2], 4);
					state = s_get_data_14;
                                }
				break;          
                      case s_get_data_14:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_14[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                     //   uart_send_str(&a_rd[2], 4);
					state = s_get_data_15;
                                }
				break;          
                      case s_get_data_15:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_15[0], 0,a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                    //    uart_send_str(&a_rd[2], 4);
					state = s_ok;
                                }
				break;          
                      case s_get_data_16:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_16[0], 0,a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                    //    uart_send_str(&a_rd[2], 4);
					state = s_get_data_1;
                                }
				break;
                        case s_get_data_17:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_17[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                     //   uart_send_str(&a_rd[2], 4);
					state = s_get_data_2;
                                }
				break;
                        case s_get_data_18:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_18[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                     //   uart_send_str(&a_rd[2], 4);
					state = s_get_data_3;
                                }
				break;
                       case s_get_data_19:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_19[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                       // uart_send_str(&a_rd[2], 4);
					state = s_get_data_4;
                                }
				break;  
                       case s_get_data_20:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_20[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                       // uart_send_str(&a_rd[2], 4);
					state = s_get_data_5;
                                }
				break;         
                       case s_get_data_21:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_21[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                      //  uart_send_str(&a_rd[2], 4);
					state = s_get_data_6;
                                }
				break;         
                       case s_get_data_22:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_22[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
//                                      //  uart_send_str(&a_rd[2], 4);
					state = s_get_data_7;
                                }
				break;         
                                
                       case s_get_data_23:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_23[0], 0,a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
//                                        //uart_send_str(&a_rd[2], 4);
					state = s_get_data_8;
                                }
				break;         
                       case s_get_data_24:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_24[0], 0,a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
//                                        uart_send_str(&a_rd[2], 4);
					state = s_get_data_9;
                                }
				break;         
                       case s_get_data_25:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_25[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
//                                        uart_send_str(&a_rd[2], 4);
					state = s_get_data_10;
				break;   
                       case s_get_data_26:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_26[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
//                                        uart_send_str(&a_rd[2], 4);
					state = s_get_data_11;
                                }
				break;         
                       case s_get_data_27:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_27[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
//                                        uart_send_str(&a_rd[2], 4);
					state = s_get_data_12;
                                }
				break;         
                       case s_get_data_28:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_28[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
//                                        uart_send_str(&a_rd[2], 4);
					state = s_get_data_13;
                                }
				break;         
                      case s_get_data_29:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_29[0], 0,a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
//                                        uart_send_str(&a_rd[2], 4);
					state = s_get_data_14;
                                }
				break;          
                      case s_get_data_30:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_30[0], 0,a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
//                                        uart_send_str(&a_rd[2], 4);
					state = s_get_data_15;
                                }
				break;          
                      case s_get_data_31:
				if (!tms37206_pwm_wr_ls_rd(2, &a_wr_31[0],0, a_rd))
					state = s_exit;
				else if (tms37206_calc_crc16(&a_rd[1], 7) != (a_rd[8] | ((uint16_t)a_rd[9]<<8)))
		          		state = s_exit;
				else
                                {
                                      //  uart_send_str(&a_rd[2], 4);
					state = s_ok;
                                }
				break;      
			case s_ok:
			//	led_on();
				//uart_send_str(&a_rd[1], 6);
				lcd_print_hex(30, 40,200,24,24, &a_rd[1], 6);
                                //lcd_print_str(0, 1, "Read OK", 7);
			  LCD_ShowString(30,40,200,24,24,"Read OK",7);
				i_fail = 0;
				state = s_wait_out;
				break;
			case s_wait_out:
				if (tms37206_pwm_wr_ls_rd(2, &a_wr_0[0], 0,a_rd))
					i_fail = 0;
				else
					i_fail++;

				if (i_fail == 2) {
				//	led_off();
					lcd_print_blank(1);
					state = s_exit;
				}
				break;
			default :
				state = s_exit;
				break;
		}
	}

	return result;
}
//*********************************************************************//
//按键读AES标签
//*********************************************************************//
bool tms37206_find_tag(tms37206_tag_t *p_tag)
{     
        bool result=FALSE;
        
	uint8_t a_wr[2] = {0xf0,0x02};

	if (!tms37206_pwm_wr_ls_rd(2, &a_wr[0],0,p_tag->a_rd))
        {   
              p_tag->mode = e_ws21_not;
	      result = FALSE;
        }
	else if (tms37206_calc_crc16(&p_tag->a_rd[1], 7) == (p_tag->a_rd[8] | ((uint16_t)p_tag->a_rd[9]<<8)))
        {
              p_tag->mode = e_ws21_yes;
              result =   TRUE;
	}

	return result;
}

bool tms37206_keypad_interaction(tms37206_tag_t *p_tag)
{      
	uint8_t key;
        uint8_t a_rd[4];
	enum {
		s_waiting, s_show_id1,s_exit
	} state;	
        if(p_tag->mode == e_ws21_not)
        return FALSE;
        

	state = s_waiting;
	while (state != s_exit) {
		switch (state) {
			case s_waiting:
				LCD_Clear(BLACK);
				lcd_print_str(0, 0, str_h_tag, 6);
				lcd_print_str(0, 1, str_read_r, 6);
                                delay_us(2500*1000);
				state = s_show_id1;
				break;
			case s_show_id1:
				LCD_Clear(BLACK);
                                a_rd[0] = p_tag->a_rd[5];
                                a_rd[1] = p_tag->a_rd[4];
                                a_rd[2] = p_tag->a_rd[3];
                                a_rd[3] = p_tag->a_rd[2];
				lcd_print_hex(30, 40,200,24,24, &a_rd[0], 4);
				LCD_ShowString(30, 70,200,24,24, str_exit_r, 6);
				while ((key = KEY_Scan(0)) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_RD_PRES)
					state = s_exit;
				break;
			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}
	
	return   TRUE;
}

//*********************************************************************//
//上位机读取AES标签
//
//
//*********************************************************************//
uint8_t tms37206_comm_handle(const uint8_t *p_ask, uint8_t *p_answer)
{
	uint8_t len_send = 1;
	p_answer[0] = 0x00;
	
	/* PWM without CRC */
	if (p_ask[0] == 0x01) {
          	if (tms37206_pwm_wr_ls_rd(p_ask[1], &p_ask[2], p_ask[2+p_ask[1]],&p_answer[1])) {
			len_send = 1 + 14;
                        p_answer[0] = p_ask[0];
		}
        }

	/* PWM add CRC */
	if (p_ask[0] == 0x02) {
		if (tms37206_pwm_wr_ls_crc_rd(p_ask[1], &p_ask[2],p_ask[2+p_ask[1]] ,&p_answer[1])) {
			len_send = 1 + 14;
                        p_answer[0] = p_ask[0];
		}
	}

      /*  if (len_send != 1) {
		// remove b9wk64 tag case 
		if (tms37206_calc_crc16(&p_answer[2], 5) == (p_answer[7] | ((uint16_t)p_answer[8]<<8))) {
			len_send = 1;
			p_answer[0] = 0x00;
		}
		// remove CRC invalid case 
		else if (tms37206_calc_crc16(&p_answer[2], 7) != (p_answer[9] | ((uint16_t)p_answer[10]<<8))) {
			len_send = 1;
			p_answer[0] = 0x00;
		}
	}
*/
		
	return len_send;
}

