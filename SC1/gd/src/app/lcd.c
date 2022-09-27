#define STR_DEFINE
#define   LCD_ST7735S_EN          (1)
//#define LCD1602_ENABLE					(1)
//#define LCD12232_ENABLE					(!LCD1602_ENABLE)
#define STR_ENGLISH						(LCD_ST7735S_EN||LCD1602_ENABLE)
#define STR_CHINESE						(0)
#include "headfile.h"


	
void lcd_print_hex(uint8_t x, uint8_t y,uint16_t width,uint16_t height,uint8_t size, const uint8_t *p_hex, uint8_t bytes)
{
	uint8_t a_ch[LCD_ROW_LENGTH];
	
	hex_to_char(p_hex, a_ch, bytes);
	LCD_ShowString(x, y,width,height,size,a_ch, bytes*2);
}
	
void lcd_print_bcd(uint8_t x, uint8_t y, uint8_t byte)
{
	uint8_t a_str[3];

	a_str[0] = byte/100 + '0';
	a_str[1] = (byte%100)/10 + '0';
	a_str[2] = byte%10 + '0';
	LCD_ShowString(x, y,200,16,16, a_str,3);
}

void lcd_print_ready_press(void)
{
#if !LCD_ST7735S_EN	
	lcd_print_str(0, 0, str_ready_rd,sizeof(str_ready_rd));
	//lcd_print_str(0, 1, str_press_rd, sizeof(str_press_rd));
#else
	LCD_Clear(BACK_COLOR); 
	LCD_ShowString(30,120,200,24,24,str_ready_rd,LCD_ROW_LENGTH);
#endif
	
}

void lcd_print_ng_press(void)
{
	#if !LCD_ST7735S_EN	
	lcd_print_str(0, 0, str_read_ng, LCD_ROW_LENGTH);
	lcd_print_str(0, 1, str_press_rd, LCD_ROW_LENGTH);
	#else
	{
		//LCD_Clear(BACK_COLOR); 
		LCD_Fill(10,50,lcddev.width,200,BACK_COLOR);
		LCD_ShowString(30,120,200,24,24,str_read_ng,LCD_ROW_LENGTH);
	}
	#endif
}

void lcd_print_reading(void)
{
#if !LCD_ST7735S_EN
	lcd_print_str(0, 0, str_reading, LCD_ROW_LENGTH);
	lcd_print_str(0, 1, str_dot8, LCD_ROW_LENGTH);
#else
	//LCD_Clear(BACK_COLOR); 
	LCD_ShowString(3,33,lcddev.width,12,12, str_reading,LCD_ROW_LENGTH);
	//LCD_ShowString(10,70,200,16,16, str_dot8,LCD_ROW_LENGTH);
#endif
}
void lcd_print_writing(void)
{
#if !LCD_ST7735S_EN
	lcd_print_str(0, 0, str_writing, LCD_ROW_LENGTH);
	lcd_print_str(0, 1, str_dot8, LCD_ROW_LENGTH);
#else
	//LCD_Clear(BACK_COLOR); 
	LCD_ShowString(5,33,lcddev.width,12,12, str_writing,LCD_ROW_LENGTH);
//	LCD_ShowString(10,70,200,16,16, str_dot8,LCD_ROW_LENGTH);
#endif
}
/*
void lcd_print_insert(void)
{
	lcd_clear_screen();
	lcd_print_str(0, 0, str_insert, 6);
	lcd_print_str(0, 1, str_exit_r, 6);
	lcd_print_str(8, 1, str_copy_c, 6);
}
*/
void lcd_print_copying(void)
{
#if !LCD_ST7735S_EN	
	lcd_clear_screen();
	lcd_print_str(0, 0, str_copying, 7);
#else
  LCD_Clear(BACK_COLOR); 
	LCD_ShowString(30,40,200,24,24, str_copying,LCD_ROW_LENGTH);
#endif
}

void lcd_print_cracking(void)
{
	//LCD_Clear(BACK_COLOR); 
#if LCD_ST7735S_EN
	//LCD_ShowString(10,90,200,16,16,"             ",LCD_ROW_LENGTH);
	LCD_ShowString(10,90,200,16,16,str_cracking,LCD_ROW_LENGTH);
#else
	lcd_print_str(0, 0, str_cracking, 8);
#endif
	#if DISPLAY_CRACK_TIME
#if LCD_ST7735S_EN
	LCD_ShowString(30,60,200,16,16,str_time, 9);
#else
	lcd_print_str(0, 1, str_time, 9);
#endif
	t2_tick_16ms = 0;
	t2_tick_1s = 0;
	timer2_ov_interrupt_en();
	timer2_start_1024();
	#endif
}

void lcd_print_crack_end(void)
{
	timer2_ov_interrupt_dis();
	timer2_stop();
	LCD_Clear(BACK_COLOR);
}

void lcd_print_blank(uint8_t y)
{
	if (y < 2)
		LCD_ShowString(30, y,200,16,16,"                ",16);
}

