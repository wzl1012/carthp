#include "headfile.h"
#define  OLED_CMD  0x00
extern uint8_t temp[4];
static void OLED_1306_iic_write(uint8_t data,uint8_t wtype)
{
	uint8_t s_data[2]={0};
	  s_data[0]=wtype;
	  s_data[1]=data;
	i2c_write(I2C1,s_data,2);
}

void oled1306_init(void)
{
	delay_us(100000);
	OLED_1306_iic_write(0xAE,OLED_CMD);//关闭显示
	
	OLED_1306_iic_write(0x40,OLED_CMD);//---set low column address
	OLED_1306_iic_write(0xB0,OLED_CMD);//---set high column address

	OLED_1306_iic_write(0xC8,OLED_CMD);//-not offset

	OLED_1306_iic_write(0x81,OLED_CMD);//设置对比度
	OLED_1306_iic_write(0xff,OLED_CMD);

	OLED_1306_iic_write(0xa1,OLED_CMD);//段重定向设置

	OLED_1306_iic_write(0xa6,OLED_CMD);//
	
	OLED_1306_iic_write(0xa8,OLED_CMD);//设置驱动路数
	OLED_1306_iic_write(0x1f,OLED_CMD);
	
	OLED_1306_iic_write(0xd3,OLED_CMD);
	OLED_1306_iic_write(0x00,OLED_CMD);
	
	OLED_1306_iic_write(0xd5,OLED_CMD);
	OLED_1306_iic_write(0xf0,OLED_CMD);
	
	OLED_1306_iic_write(0xd9,OLED_CMD);
	OLED_1306_iic_write(0x22,OLED_CMD);
	
	OLED_1306_iic_write(0xda,OLED_CMD);
	OLED_1306_iic_write(0x02,OLED_CMD);
	
	OLED_1306_iic_write(0xdb,OLED_CMD);
	OLED_1306_iic_write(0x49,OLED_CMD);
	
	OLED_1306_iic_write(0x8d,OLED_CMD);
	OLED_1306_iic_write(0x14,OLED_CMD);
	
	OLED_1306_iic_write(0xaf,OLED_CMD);
	OLED_Clear();
	OLED_ShowString(0,0,DIS_V,16,16);
	OLED_ShowString(0,2,"SYZ",16,3);
	oled_print_hex(24,2,16,temp,4);
	delay_us(1500000);
}




 	  
	   			 




