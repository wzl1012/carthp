#include "sys.h"
//#include "gd32f10x_it.h
#include "headfile.h"
////////////////////////////////////////////////////////////////////////////////// 	 
 
	   
//static uint16_t fac_ms=0;							//ms��ʱ������,��ucos��,����ÿ�����ĵ�ms��
	
	
extern void xPortSysTickHandler(void);

//systick�жϷ�����,ʹ��ucosʱ�õ�
void SysTick_Handler(void)
{	
    if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)//ϵͳ�Ѿ�����
    {
        xPortSysTickHandler();	
    }
}

							    










































