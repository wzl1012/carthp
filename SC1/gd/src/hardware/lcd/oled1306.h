#ifndef  OLED_1306_H
#define  OLED_1306_H
#include "headfile.h"
#define I2C0_SLAVE_ADDRESS7  0x78
#define OLED_DATA   0x40 
#define OLED_CMD  0x00
#define Max_Column	128
#define Max_Row		32



void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t Char_Size,uint8_t len);
void oled_print_hex(uint8_t x, uint8_t y,uint8_t size, const uint8_t *p_hex, uint8_t bytes);
extern void oled1306_init(void);
void OLED_Clear(void); 
void OLED_Display_Off(void);
void OLED_Display_On(void);
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,const unsigned char BMP[]);
void OLED_Fill(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t dot);
void OLED_drawline(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t dot);
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size2);
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t);
void OLED_DrawxsPoint(uint8_t x,uint8_t y,uint8_t t);
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t Char_Size);
void extern errchk(void);
#endif


