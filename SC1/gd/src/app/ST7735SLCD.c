#include "headfile.h"
#include "FONT.h"
uint16_t POINT_COLOR=0xFFFF;     //
uint16_t BACK_COLOR=0x0000;      // 
_lcd_dev lcddev={0};
static void write_command(uint8_t cmd)
{
	  LCD_RS_CLR;//write cmd
    LCD_CS_CLR;
    LCD_DATAOUT(cmd);
    LCD_WR_CLR;
    LCD_WR_SET;
    LCD_CS_SET; 
	
}
static void write_parameter(uint8_t data)
{
	      LCD_RS_SET;  //write data
        LCD_CS_CLR;
        LCD_DATAOUT(data);
        LCD_WR_CLR;
        LCD_WR_SET;
        LCD_CS_SET;	
}
void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue)
{
	    if(lcddev.id == 0X7735){
         write_command((uint8_t) LCD_Reg);  
         write_parameter((uint8_t)LCD_RegValue);
			}else{
				write_command(LCD_Reg/256); 
				write_command(LCD_Reg%256); 
				write_parameter(LCD_RegValue/256);
				write_parameter(LCD_RegValue%256);
			}
}
void user_lcd_init(void)
{	

//------ Initialization Code  --------------//
//---ST7735S Reset Sequence--------//
LCD_RESET_SET;		// 
delay_us(1*1000);	// Delay 1 ms
LCD_RESET_CLR ;		// Pull the reset pin low 
delay_us(20*1000);	// Delay 10 ms
LCD_RESET_SET;		// Pull the reset pin high 
//--------------------------------End ST7735S Reset Sequence --------------------------------------//
delay_us(150*1000);	// This delay time is necessary
write_command(0x11); //Sleep out
delay_us(120*1000); //Delay 120ms
//------------------------------------ST7735S Frame Rate-----------------------------------------//
write_command(0xB1);
write_parameter(0x05);
write_parameter(0x3C);
write_parameter(0x3C);
write_command(0xB2);
write_parameter(0x05);
write_parameter(0x3C);
write_parameter(0x3C);
write_command(0xB3);
write_parameter(0x05);
write_parameter(0x3C);
write_parameter(0x3C);
write_parameter(0x05);
write_parameter(0x3C);
write_parameter(0x3C);
//------------------------------------End ST7735S Frame Rate-----------------------------------------//
write_command(0xB4); //Dot inversion
write_parameter(0x03);
//------------------------------------ST7735S Power Sequence-----------------------------------------//
write_command(0xC0);
write_parameter(0x28);
write_parameter(0x08);
write_parameter(0x04);
write_command(0xC1);
write_parameter(0XC0);
write_command(0xC2);
write_parameter(0x0D);
write_parameter(0x00);
write_command(0xC3);
write_parameter(0x8D);
write_parameter(0x2A);
write_command(0xC4);
write_parameter(0x8D);
write_parameter(0xEE);
//---------------------------------End ST7735S Power Sequence-------------------------------------//
write_command(0xC5); //VCOM
write_parameter(0x12);

//write_command(0xC7); //VCOM
//write_parameter(0x1f);

write_command(0x36); //MX, MY, RGB mode
write_parameter(0xC8);//00,08  
//------------------------------------ST7735S Gamma Sequence-----------------------------------------//
write_command(0xE0);
write_parameter(0x04);
write_parameter(0x22);
write_parameter(0x07);
write_parameter(0x0A);
write_parameter(0x2E);
write_parameter(0x30);
write_parameter(0x25);
write_parameter(0x2A);
write_parameter(0x28);
write_parameter(0x26);
write_parameter(0x2E);
write_parameter(0x3A);
write_parameter(0x00);
write_parameter(0x01);
write_parameter(0x03);
write_parameter(0x13);
write_command(0xE1);
write_parameter(0x04);
write_parameter(0x16);
write_parameter(0x06);
write_parameter(0x0D);
write_parameter(0x2D);
write_parameter(0x26);
write_parameter(0x23);
write_parameter(0x27);
write_parameter(0x27);
write_parameter(0x25);
write_parameter(0x2D);
write_parameter(0x3B);
write_parameter(0x00);
write_parameter(0x01);
write_parameter(0x04);
write_parameter(0x13);
//------------------------------------End ST7735S Gamma Sequence-----------------------------------------//
write_command(0x3A); //65k mode
write_parameter(0x05);
/*
write_command(0x2A);	//X Address Setup
write_parameter(0x00);
write_parameter(0x00);
write_parameter(0x00);
write_parameter(0x83);

write_command(0x2B);	//Y Address Setup
write_parameter(0x00);
write_parameter(0x00);
write_parameter(0x00);
write_parameter(0xA1);*/

write_command(0x29); //Display on
//write_command(0x2C);
LCD_Display_Dir(0);
LCD_BL_SET;
LCD_Clear(BLACK);
LCD_ShowString(3,3,lcddev.width,12,12, str_mode_name,sizeof(str_mode_name));
LCD_ShowString(3,18,lcddev.width,12,12, str_sw_version,sizeof(str_sw_version));
}

void sleep_in()
{
    write_command(0x10);	//Normal-Mode -> Sleep In-Mode
    delay_us(150);
}

void sleep_out()
{
    write_command(0x11);	//Sleep In-Mode -> Normal-Mode
	delay_us(150*1000);
}

void LCD_BlockWrite(uint16_t startx,uint16_t starty,uint16_t endx,uint16_t endy)
{
	
	write_command(0x2A);	//X Address Setup
	write_parameter(0x00);
	write_parameter(startx);
	write_parameter(0x00);
	write_parameter(endx);

	write_command(0x2B);	//Y Address Setup
	write_parameter(0x00);
	write_parameter(starty);
	write_parameter(0x00);
	write_parameter(endy);

	write_command(0x2C);	//Write Display Data

}

/*
void LCD_blockClear(uint16_t startx,uint16_t starty,uint16_t endx,uint16_t endy, uint16_t data)
{
	uint16_t LCD_x;
	uint16_t LCD_y;

	write_command(0x2A);	//X Address Setup
	write_parameter(0x00);
	write_parameter(startx);
	write_parameter(0x00);
	write_parameter(endx);

	write_command(0x2B);	//Y Address Setup
	write_parameter(0x00);
	write_parameter(starty);
	write_parameter(0x00);
	write_parameter(endy);

	write_command(0x2C);	//Write Display Data

	for(LCD_y=y1; LCD_y<=y2; LCD_y++)
	{
		for(LCD_x=x1; LCD_x<=x2; LCD_x++)
		{

		#ifdef LCD_16BIT_MODE
		 	LCD_DataWrite(data);
		#else
		 *(volatile uint8_t *)LCD_DATA_ADDR = (data&0xFF00)>>8;
		 delay_us(1000);
		 *(volatile uint8_t *)LCD_DATA_ADDR= data&0xFF;
		#endif
		}
	}
}*/
//???GRAM
void LCD_WriteRAM_Prepare(void)
{
    write_command(lcddev.wramcmd);
} 
//LCD?GRAM
//RGB_Code:???
void LCD_WriteRAM(uint16_t RGB_Code)
{
    write_parameter(RGB_Code/256);      //????GRAM
	  write_parameter(RGB_Code%256);  
}
//??????
//Xpos:???
//Ypos:???
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
    if (lcddev.id == 0X1963)
    {
        if (lcddev.dir == 0)   //x??????
        {
            Xpos = lcddev.width - 1 - Xpos;
            write_command(lcddev.setxcmd);
            write_parameter(0);
            write_parameter(0);
            write_parameter(Xpos >> 8);
            write_parameter(Xpos & 0XFF);
        }
        else
        {
            write_command(lcddev.setxcmd);
            write_parameter(Xpos >> 8);
            write_parameter(Xpos & 0XFF);
            write_parameter((lcddev.width - 1) >> 8);
            write_parameter((lcddev.width - 1) & 0XFF);
        }

        write_command(lcddev.setycmd);
        write_parameter(Ypos >> 8);
        write_parameter(Ypos & 0XFF);
        write_parameter((lcddev.height - 1) >> 8);
        write_parameter((lcddev.height - 1) & 0XFF);

    }
    else if (lcddev.id == 0X5510)
    {
        write_command(lcddev.setxcmd);
        write_parameter(Xpos >> 8);
        write_command(lcddev.setxcmd + 1);
        write_parameter(Xpos & 0XFF);
        write_command(lcddev.setycmd);
        write_parameter(Ypos >> 8);
        write_command(lcddev.setycmd + 1);
        write_parameter(Ypos & 0XFF);
    }
    else     //9341/5310/7789?????
    {
        write_command(lcddev.setxcmd);
        write_parameter(Xpos >> 8);
        write_parameter(Xpos & 0XFF);
        write_command(lcddev.setycmd);
        write_parameter(Ypos >> 8);
        write_parameter(Ypos & 0XFF);
    }
}

//??LCD???????
//dir:0~7,??8???(?????lcd.h)
//9341/5310/5510/1963/7789?IC??????
//??:?????????????????(???9341),
//??,?????L2R_U2D??,???????????,?????????.
void LCD_Scan_Dir(uint8_t dir)
{
    uint16_t regval = 0;
    uint16_t dirreg = 0;
    uint16_t temp;

    //???,?1963???????, ??IC??????!???1963????, ??IC???????
    if ((lcddev.dir == 1 && lcddev.id != 0X1963) || (lcddev.dir == 0 && lcddev.id == 0X1963))
    {
        switch (dir)   //????
        {
            case 0:
                dir = 6;
                break;

            case 1:
                dir = 7;
                break;

            case 2:
                dir = 4;
                break;

            case 3:
                dir = 5;
                break;

            case 4:
                dir = 1;
                break;

            case 5:
                dir = 0;
                break;

            case 6:
                dir = 3;
                break;

            case 7:
                dir = 2;
                break;
        }
    }

    switch (dir)
    {
        case L2R_U2D://????,????
            regval |= (0 << 7) | (0 << 6) | (0 << 5);
            break;

        case L2R_D2U://????,????
            regval |= (1 << 7) | (0 << 6) | (0 << 5);
            break;

        case R2L_U2D://????,????
            regval |= (0 << 7) | (1 << 6) | (0 << 5);
            break;

        case R2L_D2U://????,????
            regval |= (1 << 7) | (1 << 6) | (0 << 5);
            break;

        case U2D_L2R://????,????
            regval |= (0 << 7) | (0 << 6) | (1 << 5);
            break;

        case U2D_R2L://????,????
            regval |= (0 << 7) | (1 << 6) | (1 << 5);
            break;

        case D2U_L2R://????,????
            regval |= (1 << 7) | (0 << 6) | (1 << 5);
            break;

        case D2U_R2L://????,????
            regval |= (1 << 7) | (1 << 6) | (1 << 5);
            break;
    }

    if (lcddev.id == 0X5510)dirreg = 0X3600;
    else dirreg = 0X36;

    if (lcddev.id == 0X9341 || lcddev.id == 0X7789)   //9341 & 7789 ???BGR?
    {
        regval |= 0X08;
    }
			LCD_WriteReg(dirreg,regval);
    if (lcddev.id != 0X1963)   //1963??????
    {
        if (regval & 0X20)
        {
            if (lcddev.width < lcddev.height)   //??X,Y
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
        else
        {
            if (lcddev.width > lcddev.height)   //??X,Y
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
    }

    //??????(??)??
    if (lcddev.id == 0X5510)
    {
        write_command(lcddev.setxcmd);
        write_parameter(0);
        write_command(lcddev.setxcmd + 1);
        write_parameter(0);
        write_command(lcddev.setxcmd + 2);
        write_parameter((lcddev.width - 1) >> 8);
        write_command(lcddev.setxcmd + 3);
        write_parameter((lcddev.width - 1) & 0XFF);
        write_command(lcddev.setycmd);
        write_parameter(0);
        write_command(lcddev.setycmd + 1);
        write_parameter(0);
        write_command(lcddev.setycmd + 2);
        write_parameter((lcddev.height - 1) >> 8);
        write_command(lcddev.setycmd + 3);
        write_parameter((lcddev.height - 1) & 0XFF);
    }
    else
    {
        write_command(lcddev.setxcmd);
        write_parameter(0);
        write_parameter(0);
        write_parameter((lcddev.width - 1) >> 8);
        write_parameter((lcddev.width - 1) & 0XFF);
        write_command(lcddev.setycmd);
        write_parameter(0);
        write_parameter(0);
        write_parameter((lcddev.height - 1) >> 8);
        write_parameter((lcddev.height - 1) & 0XFF);
    }
}

//??
//x,y:??
//POINT_COLOR:?????
void LCD_DrawPoint(uint16_t x,uint16_t y)
{
    LCD_SetCursor(x, y);        //??????
    LCD_WriteRAM_Prepare();     //????GRAM
    write_parameter(POINT_COLOR/256);
	  write_parameter(POINT_COLOR%256);
}

//????
//x,y:??
//color:??
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
    if (lcddev.id == 0X5510)
    {
        write_command(lcddev.setxcmd);
        write_parameter(x >> 8);
        write_command(lcddev.setxcmd + 1);
        write_parameter(x & 0XFF);
        write_command(lcddev.setycmd);
        write_parameter(y >> 8);
        write_command(lcddev.setycmd + 1);
        write_parameter(y & 0XFF);
    }
    else if (lcddev.id == 0X1963)
    {
        if (lcddev.dir == 0)x = lcddev.width - 1 - x;

        write_command(lcddev.setxcmd);
        write_parameter(x >> 8);
        write_parameter(x & 0XFF);
        write_parameter(x >> 8);
        write_parameter(x & 0XFF);
        write_command(lcddev.setycmd);
        write_parameter(y >> 8);
        write_parameter(y & 0XFF);
        write_parameter(y >> 8);
        write_parameter(y & 0XFF);
    }
    else     //9341/5310/7789?????
    {
        write_command(lcddev.setxcmd);
        write_parameter(x >> 8);
        write_parameter(x & 0XFF);
        write_command(lcddev.setycmd);
        write_parameter(y >> 8);
        write_parameter(y & 0XFF);
    }
		 write_command(lcddev.wramcmd);
/*
    LCD_RS_CLR;
    LCD_CS_CLR;
    LCD_DATAOUT(lcddev.wramcmd);    //
    LCD_WR_CLR;
    LCD_WR_SET;
    LCD_CS_SET;*/
    write_parameter(color/256);         //
		write_parameter(color%256);
}

//SSD1963 ????
//pwm:????,0~100.????.
void LCD_SSD_BackLightSet(uint8_t pwm)
{
    write_command(0xBE);           //??PWM??
    write_parameter(0x05);          //1??PWM??
    write_parameter(pwm * 2.55);    //2??PWM???
    write_parameter(0x01);          //3??C
    write_parameter(0xFF);          //4??D
    write_parameter(0x00);          //5??E
    write_parameter(0x00);          //6??F
}

//set LCD display dir
//dir:0,V;1,H
void LCD_Display_Dir(uint8_t dir)
{
    lcddev.dir = dir;       //
    lcddev.id = 0x7735;
    if (dir == 0)           //V
    {
        lcddev.width = 132;
        lcddev.height = 162;

        if (lcddev.id == 0x5510)
        {
            lcddev.wramcmd = 0X2C00;
            lcddev.setxcmd = 0X2A00;
            lcddev.setycmd = 0X2B00;
            lcddev.width = 480;
            lcddev.height = 800;
        }
        else if (lcddev.id == 0X1963)
        {
            lcddev.wramcmd = 0X2C;  //????GRAM???
            lcddev.setxcmd = 0X2B;  //???X????
            lcddev.setycmd = 0X2A;  //???Y????
            lcddev.width = 480;     //????480
            lcddev.height = 800;    //????800
        }
        else                        //??IC, ??: 9341 / 5310 / 7789?IC
        {
            lcddev.wramcmd = 0X2C;
            lcddev.setxcmd = 0X2A;
            lcddev.setycmd = 0X2B;
        }

        if (lcddev.id == 0X5310)    //???5310 ???? 320*480???
        {
            lcddev.width = 320;
            lcddev.height = 480;
        }
    }
    else     //??
    {
        lcddev.width = 162;
        lcddev.height = 132;

        if (lcddev.id == 0x5510)
        {
            lcddev.wramcmd = 0X2C00;
            lcddev.setxcmd = 0X2A00;
            lcddev.setycmd = 0X2B00;
            lcddev.width = 800;
            lcddev.height = 480;
        }
        else if (lcddev.id == 0X1963)
        {
            lcddev.wramcmd = 0X2C;  //????GRAM???
            lcddev.setxcmd = 0X2A;  //???X????
            lcddev.setycmd = 0X2B;  //???Y????
            lcddev.width = 800;     //????800
            lcddev.height = 480;    //????480
        }
        else                        //??IC, ??: 9341 / 5310 / 7789?IC
        {
            lcddev.wramcmd = 0X2C;
            lcddev.setxcmd = 0X2A;
            lcddev.setycmd = 0X2B;
        }

        if (lcddev.id == 0X5310)    //???5310 ???? 320*480???
        {
            lcddev.width = 480;
            lcddev.height = 320;
        }
    }

    LCD_Scan_Dir(DFT_SCAN_DIR);     //??????
}

//????,???????????????(sx,sy).
//sx,sy:??????(???)
//width,height:???????,????0!!
//????:width*height. 
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
{    
    uint16_t twidth, theight;
    twidth = sx + width - 1;
    theight = sy + height - 1;

    if (lcddev.id == 0X1963 && lcddev.dir != 1)     //1963??????
    {
        sx = lcddev.width - width - sx;
        height = sy + height - 1;
        write_command(lcddev.setxcmd);
        write_parameter(sx >> 8);
        write_parameter(sx & 0XFF);
        write_parameter((sx + width - 1) >> 8);
        write_parameter((sx + width - 1) & 0XFF);
        write_command(lcddev.setycmd);
        write_parameter(sy >> 8);
        write_parameter(sy & 0XFF);
        write_parameter(height >> 8);
        write_parameter(height & 0XFF);
    }
    else if (lcddev.id == 0X5510)
    {
        write_command(lcddev.setxcmd);
        write_parameter(sx >> 8);
        write_command(lcddev.setxcmd + 1);
        write_parameter(sx & 0XFF);
        write_command(lcddev.setxcmd + 2);
        write_parameter(twidth >> 8);
        write_command(lcddev.setxcmd + 3);
        write_parameter(twidth & 0XFF);
        write_command(lcddev.setycmd);
        write_parameter(sy >> 8);
        write_command(lcddev.setycmd + 1);
        write_parameter(sy & 0XFF);
        write_command(lcddev.setycmd + 2);
        write_parameter(theight >> 8);
        write_command(lcddev.setycmd + 3);
        write_parameter(theight & 0XFF);
    }
    else     //9341/5310/7789/1963?? ? ????
    {
        write_command(lcddev.setxcmd);
        write_parameter(sx >> 8);
        write_parameter(sx & 0XFF);
        write_parameter(twidth >> 8);
        write_parameter(twidth & 0XFF);
        write_command(lcddev.setycmd);
        write_parameter(sy >> 8);
        write_parameter(sy & 0XFF);
        write_parameter(theight >> 8);
        write_parameter(theight & 0XFF);
    }
}
void LCD_Clear(uint16_t color)
{
    uint32_t index = 0;
    uint32_t totalpoint = lcddev.width;
    totalpoint *= lcddev.height;    //?????

    LCD_SetCursor(0x00, 0x00);    //??????
    LCD_WriteRAM_Prepare();         //????GRAM

    for (index = 0; index < totalpoint; index++)
    {
			  write_parameter(color/256);
        write_parameter(color%256);
    }
}

//????????????
//????:(xend-xsta+1)*(yend-ysta+1)
//xsta
//color:??????
void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)
{
    uint16_t i, j;
    uint16_t xlen = 0;

    xlen = ex - sx + 1;

    for (i = sy; i <= ey; i++)
    {
        LCD_SetCursor(sx, i);       //??????
        LCD_WriteRAM_Prepare();     //????GRAM

        for (j = 0; j < xlen; j++)
        {
            write_parameter(color/256);     //??????
					  write_parameter(color%256);
        }
    }
}

//?????????????
//(sx,sy),(ex,ey):????????,?????:(ex-sx+1)*(ey-sy+1)
//color:??????
void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color)
{
    uint16_t height, width;
    uint16_t i, j;
    width = ex - sx + 1;            //???????
    height = ey - sy + 1;           //??

    for (i = 0; i < height; i++)
    {
        LCD_SetCursor(sx, sy + i);  //??????
        LCD_WriteRAM_Prepare();     //????GRAM

        for (j = 0; j < width; j++)
        {
            write_parameter(color[i * width + j]/256); //????
					  write_parameter(color[i * width + j]%256); //????
        }
    }
}

//??
//x1,y1:????
//x2,y2:????  
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1;              //
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;

    if (delta_x > 0)incx = 1;       //
    else if (delta_x == 0)incx = 0; //
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0)incy = 1;
    else if (delta_y == 0)incy = 0; //???
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }

    if ( delta_x > delta_y)distance = delta_x; //?????????
    else distance = delta_y;

    for (t = 0; t <= distance + 1; t++ )    //????
    {
        LCD_DrawPoint(uRow, uCol); //??
        xerr += delta_x ;
        yerr += delta_y ;

        if (xerr > distance)
        {
            xerr -= distance;
            uRow += incx;
        }

        if (yerr > distance)
        {
            yerr -= distance;
            uCol += incy;
        }
    }
}

//???
//(x1,y1),(x2,y2):???????
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    LCD_DrawLine(x1, y1, x2, y1);
    LCD_DrawLine(x1, y1, x1, y2);
    LCD_DrawLine(x1, y2, x2, y2);
    LCD_DrawLine(x2, y1, x2, y2);
}

//??????????????
//(x,y):???
//r    :??
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r)
{
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1);       //??????????

    while (a <= b)
    {
        LCD_DrawPoint(x0 + a, y0 - b);        //5
        LCD_DrawPoint(x0 + b, y0 - a);        //0
        LCD_DrawPoint(x0 + b, y0 + a);        //4
        LCD_DrawPoint(x0 + a, y0 + b);        //6
        LCD_DrawPoint(x0 - a, y0 + b);        //1
        LCD_DrawPoint(x0 - b, y0 + a);
        LCD_DrawPoint(x0 - a, y0 - b);        //2
        LCD_DrawPoint(x0 - b, y0 - a);        //7
        a++;

        //??Bresenham????
        if (di < 0)di += 4 * a + 6;
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

//???????????
//x,y:????
//num:??????:" "--->"~"
//size:???? 12/16/24
//mode:????(1)???????(0)
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode)
{
    uint8_t temp, t1, t;
    uint16_t y0 = y;
    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);  //???????????????????
    num = num - ' ';    //???????(ASCII??????????,??-' '?????????)

    for (t = 0; t < csize; t++)
    {
        if (size == 12)temp = asc2_1206[num][t];        //??1206??
        else if (size == 16)temp = asc2_1608[num][t];   //??1608??
        else if (size == 24)temp = asc2_2412[num][t];   //??2412??
        else return;                                    //?????

        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)LCD_Fast_DrawPoint(x, y, POINT_COLOR);
            else if (mode == 0)LCD_Fast_DrawPoint(x, y, BACK_COLOR);

            temp <<= 1;
            y++;

            if (y >= lcddev.height)return;      //????

            if ((y - y0) == size)
            {
                y = y0;
                x++;

                if (x >= lcddev.width)return;   //????

                break;
            }
        }
    } 
}

//m^n??
//???:m^n??.
uint32_t LCD_Pow(uint8_t m,uint8_t n)
{
    uint32_t result=1;
    while(n--)result*=m;    
    return result;
}

//????,???0,????
//x,y :????	 
//len :?????
//size:????
//color:?? 
//num:??(0~4294967295);
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size)
{
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++)
    {
        temp = (num / LCD_Pow(10, len - t - 1)) % 10;

        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                LCD_ShowChar(x + (size / 2)*t, y, ' ', size, 0);
                continue;
            }
            else enshow = 1;

        }

        LCD_ShowChar(x + (size / 2)*t, y, temp + '0', size, 0);
    }
}

//????,???0,????
//x,y:????
//num:??(0~999999999);
//len:??(???????)
//size:????
//mode:
//[7]:0,???;1,??0.
//[6:1]:??
//[0]:0,?????;1,????.
void LCD_ShowxNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode)
{  
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++)
    {
        temp = (num / LCD_Pow(10, len - t - 1)) % 10;

        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                if (mode & 0X80)LCD_ShowChar(x + (size / 2)*t, y, '0', size, mode & 0X01);
                else LCD_ShowChar(x + (size / 2)*t, y, ' ', size, mode & 0X01);

                continue;
            }
            else enshow = 1;

        }

        LCD_ShowChar(x + (size / 2)*t, y, temp + '0', size, mode & 0X01);
    }
}

//?????
//x,y:????
//width,height:????
//size:????
//*p:???????
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,const uint8_t *p,uint8_t len)
{         
    uint8_t x0 = x;
    width += x;
    height += y;

    while ((*p <= '~') && (*p >= ' ')&&len--)   //?????????!
    {
        if (x >= width)
        {
            x = x0;
            y += size;
        }

        if (y >= height)break; //??

        LCD_ShowChar(x, y, *p, size, 0);
        x += size / 2;
        p++;
    }
}






























