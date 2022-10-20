#ifndef OLED_DIS_H
#define OLED_DIS_H
#include "headfile.h"
#define  DIS_V ("K3_Genie V1.0.0")
extern void OLED_Clear(void);
extern void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t Char_Size);
extern void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size2);
extern void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t Char_Size);
extern void  OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t Char_Size,uint8_t len);
extern void OLED_ShowStr(uint8_t x,uint8_t y,const uint8_t *p,uint8_t size);
extern void oled_print_hex(uint8_t x, uint8_t y,uint8_t size, const uint8_t *p_hex, uint8_t bytes);
extern void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,const unsigned char BMP[]);
extern void OLED_drawline(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t dot);
extern void OLED_Display_Off(void);
extern void OLED_Display_On(void);
extern void i2c_write(uint32_t i2cdev,uint8_t *data,uint8_t bytes);
extern void oled1306_init(void);
void oled_print_reading(void);
void oled_print_writing(void);
void oled_print_ng_press(void);
void oled_print_RFrate(void);


#endif



