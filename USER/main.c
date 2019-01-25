#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "vl53l0x.h"
#include "log.h"
#include "msg.h"
#include "uart2.h"
#include "uart3.h"
#include "uart4.h"


//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define LED0_TASK_PRIO		2
//任务堆栈大小	
#define LED0_STK_SIZE 		50  
//任务句柄
TaskHandle_t LED0Task_Handler;
//任务函数
void led0_task(void *pvParameters);

//任务优先级
#define VL53L0X0_TASK_PRIO		3
//任务堆栈大小	
#define VL53L0X0_STK_SIZE 		1024
//任务句柄
TaskHandle_t VL53L0X0Task_Handler;
//任务函数
void vl53l0x0_task(void *pvParameters);

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4	 
    delay_init();	    				//延时函数初始化	  
    uart_init(115200);					//初始化串口
    LED_Init();		  					//初始化LED
    UART2_Init();
    UART3_Init();
    UART4_Init();


    //初始时msg系统
    LinkListInit();

     
    //创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();          //开启任务调度
}

//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
    //创建LED0任务
    xTaskCreate((TaskFunction_t )led0_task,     	
                (const char*    )"led0_task",   	
                (uint16_t       )LED0_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )LED0_TASK_PRIO,	
                (TaskHandle_t*  )&LED0Task_Handler);   
    //创建VL53L0X_0任务
    xTaskCreate((TaskFunction_t )vl53l0x0_task,     
                (const char*    )"vl53l0x_0_task",   
                (uint16_t       )VL53L0X0_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )VL53L0X0_TASK_PRIO,
                (TaskHandle_t*  )&VL53L0X0Task_Handler);         
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}

//LED0任务函数 
void led0_task(void *pvParameters)
{
    int led_state = 1;
    char buff[20] = "1234\n";
    LED0 = 0;
    while(1)
    {
        LED0=~LED0;
        LOGD("hello world 0");
        
        snprintf(buff, 19, "I'm uart3\r\n");
        UART3_Send_Bytes((u8*)buff, strlen(buff));

        snprintf(buff, 19, "I'm uart4\r\n");
        UART4_Send_Bytes((u8*)buff, strlen(buff));

        snprintf(buff, 19, "I'm uart2\r\n");
        UART2_Send_Bytes((u8*)buff, strlen(buff));
        
        vTaskDelay(1000);
        led_state = !led_state;
        MessageSend(MSG_ID_RED_LED_CONTROL, &led_state, sizeof(int), MESSAGE_IS_POINTER);
    }
}   


//创建VL53L0X_0任务
void vl53l0x0_task(void *pvParameters)
{
    VL53L0X_i2c_init();//初始化VL53L0X的IIC
    VL53L0X_begin();//初始化每个VL53L0X设备
    vl53l0x_general_start();
}
