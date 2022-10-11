#include "sys.h"
//#include "gd32f10x_it.h
#include "headfile.h"
////////////////////////////////////////////////////////////////////////////////// 	 
 
	   
//static uint16_t fac_ms=0;							//ms延时倍乘数,在ucos下,代表每个节拍的ms数
	
	
extern void xPortSysTickHandler(void);

//systick中断服务函数,使用ucos时用到
void SysTick_Handler(void)
{	
    if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)//系统已经运行
    {
        xPortSysTickHandler();	
    }
}

							    










































