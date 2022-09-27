#ifndef _ST7735S_H_
#define _ST7735S_H_ 
#include "gd32f10x.h"
//LCD parameter
typedef struct  
{
    uint16_t width;      //LCD W
    uint16_t height;     //LCD H
    uint16_t id;         //LCD ID
    uint8_t  dir;        //LCD  H/V
    uint16_t wramcmd;    //gram cmd
    uint16_t setxcmd;    //set x
    uint16_t  setycmd;   //set y
} _lcd_dev;

extern _lcd_dev lcddev; 

extern uint16_t  BACK_COLOR; //bg col
extern uint16_t  POINT_COLOR;
#define WHITE           0xFFFF
#define BLACK           0x0000
#define BLUE            0x001F
#define BRED            0XF81F
#define GRED            0XFFE0
#define GBLUE           0X07FF
#define RED             0xF800
#define MAGENTA         0xF81F
#define GREEN           0x07E0
#define CYAN            0x7FFF
#define YELLOW          0xFFE0
#define BROWN           0XBC40  //
#define BRRED           0XFC07  //
#define GRAY            0X8430  //
#define L2R_U2D  0 //????,????
#define L2R_D2U  1 //????,????
#define R2L_U2D  2 //????,????
#define R2L_D2U  3 //????,????

#define U2D_L2R  4 //????,????
#define U2D_R2L  5 //????,????
#define D2U_L2R  6 //????,????
#define D2U_R2L  7 //????,????
#define DFT_SCAN_DIR  L2R_U2D 
//st7735s lcd  define
#define LCD_BL_SET  gpio_bit_set(GPIOC, GPIO_PIN_10)  // PC10
 
#define LCD_CS_SET  gpio_bit_set(GPIOC, GPIO_PIN_9) 
#define LCD_RS_SET  gpio_bit_set(GPIOC, GPIO_PIN_8)
#define LCD_WR_SET  gpio_bit_set(GPIOC, GPIO_PIN_7)
#define LCD_RD_SET  gpio_bit_set(GPIOC, GPIO_PIN_6)

#define LCD_BL_CLR  gpio_bit_reset(GPIOC, GPIO_PIN_7)  // PC7
#define LCD_CS_CLR  gpio_bit_reset(GPIOC, GPIO_PIN_9)
#define LCD_RS_CLR  gpio_bit_reset(GPIOC, GPIO_PIN_8)
#define LCD_WR_CLR  gpio_bit_reset(GPIOC, GPIO_PIN_7)
#define LCD_RD_CLR  gpio_bit_reset(GPIOC, GPIO_PIN_6)

#define LCD_RESET_SET  gpio_bit_set(GPIOB, GPIO_PIN_8)
#define LCD_RESET_CLR  gpio_bit_reset(GPIOB, GPIO_PIN_8)
//PB0~15 lcd_data
#define LCD_DATAOUT(x) gpio_port_write(GPIOB,((gpio_input_port_get(GPIOB)&0xFF00)|x))
void user_lcd_init(void);
void LCD_Clear(uint16_t Color);                                                  //??
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);                                     //????
void LCD_DrawPoint(uint16_t x,uint16_t y);                                           //??
void LCD_Fast_DrawPoint(uint16_t x, uint16_t y, uint16_t color);                           //????
uint16_t  LCD_ReadPoint(uint16_t x, uint16_t y);                                           //??
void LCD_Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r);                                 //??
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);                          //??
void LCD_DrawRectangle(uint16_t x1,uint16_t y1, uint16_t x2, uint16_t y2);                     //???
void LCD_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color);                   //????
void LCD_Color_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color);            //??????
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t mode);                  //??????
void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size);                   //??????
void LCD_ShowxNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode);         //?? ??
void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size,const uint8_t *p,uint8_t len);   //???????,12/16??
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(uint16_t RGB_Code);
void LCD_Scan_Dir(uint8_t dir);                                  //???????
void LCD_Display_Dir(uint8_t dir);                               //????????
void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue);
void LCD_Set_Window(uint16_t sx, uint16_t sy, uint16_t width,uint16_t height); //????
#endif
