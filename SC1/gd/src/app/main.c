#include "headfile.h"

#define START_TASK_PRIO			1

#define START_STK_SIZE 			128  

TaskHandle_t StartTask_Handler;

void start_task(void *pvParameters);



#define Cardrd_TASK_PRIO 			5

#define Cardrd_STK_SIZE			512

TaskHandle_t CardrdTask_Handler;

void cardrd_task(void *pvParameters);


#define BTR_T_TASK_PRIO		4
#define BTR_T_STK_SIZE		1200
TaskHandle_t BTR_T_Task_Handler;
void BTR_T_task(void *pvParameters);
// power check and sw on/off task
#define SLEEP_TASK_PRIO		3
#define SLEEP_STK_SIZE		112
TaskHandle_t SLEEP_Task_Handler;
void SLEEP_task(void *pvParameters);

#define POWERM_TASK_PRIO		2
#define POWER_M_STK_SIZE		176
TaskHandle_t POWER_M_Task_Handler;
void POWER_M_task(void *pvParameters);

int main(void)
{	
	uint32_t tmpadr=0;
	if((tmpadr=*(uint32_t*)(FWCURRNFLSADR))!=FLS_SEC1_ADR&&tmpadr!=FLS_SEC2_ADR)
	{
		 while(1);
	}
	nvic_vector_table_set(FLSBASE, tmpadr&0xFFFFF);
	periph_init( );
    xTaskCreate((TaskFunction_t )start_task,           
                (const char*    )"start_task",          
                (uint16_t       )START_STK_SIZE,        
                (void*          )NULL,                 
                (UBaseType_t    )START_TASK_PRIO,       
                (TaskHandle_t*  )&StartTask_Handler);              
    vTaskStartScheduler();          
}
void start_task(void *pvParameters)
{
	 taskENTER_CRITICAL();         
    xTaskCreate((TaskFunction_t )cardrd_task,            
                (const char*    )"cardrd_task",         
                (uint16_t       )Cardrd_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )Cardrd_TASK_PRIO,       
                (TaskHandle_t*  )&CardrdTask_Handler);   
    xTaskCreate((TaskFunction_t )BTR_T_task,            
                (const char*    )"BTR_T_task",          
                (uint16_t       )BTR_T_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )BTR_T_TASK_PRIO,       
                (TaskHandle_t*  )&BTR_T_Task_Handler);   
    xTaskCreate((TaskFunction_t )POWER_M_task,            
                (const char*    )"POWER_M_task",         
                (uint16_t       )POWER_M_STK_SIZE,        
                (void*          )NULL,                
                (UBaseType_t    )POWERM_TASK_PRIO,     
                (TaskHandle_t*  )&POWER_M_Task_Handler);   							
   xTaskCreate((TaskFunction_t )SLEEP_task,            
                (const char*    )"SLEEP_task",         
                (uint16_t       )SLEEP_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )SLEEP_TASK_PRIO,      
                (TaskHandle_t*  )&SLEEP_Task_Handler); 
  /* xTaskCreate((TaskFunction_t )RFCK_task,            
                (const char*    )"RFCK_task",         
                (uint16_t       )RFCK_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )RFCK_TASK_PRIO,      
                (TaskHandle_t*  )&RFCK_Task_Handler);*/   							
       vTaskDelete(StartTask_Handler); 
    taskEXIT_CRITICAL();            									
}
void cardrd_task(void *pvParameters)
{
  keypad_event_check();	
}
void BTR_T_task(void *pvParameters)
{
	BT_R_check();
}
void SLEEP_task(void *pvParameters)
{	
	 sleep_mag();
}
void POWER_M_task(void *pvParameters)
{	
	power_mag();	
}

