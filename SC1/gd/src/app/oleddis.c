#include "headfile.h"

void oled_print_reading(void)
{
  OLED_Clear();
	OLED_ShowString(0,0,str_reading,24,LCD_ROW_LENGTH);
}

void oled_print_writing(void)
{
  OLED_Clear();
	OLED_ShowString(0,0,str_writing,24,LCD_ROW_LENGTH);
}
void oled_print_ng_press(void)
{
    OLED_Clear();
		OLED_ShowString(0,0,str_read_ng,24,LCD_ROW_LENGTH);
}

void oled_print_RFrate(void)
{
  OLED_Clear();
	OLED_ShowNum(40,0,0,3,24);
	OLED_ShowChar(73,0,'.',24);
	//OLED_DrawxsPoint(64,1,1);
	OLED_ShowNum(78,0,0,2,24);
}

