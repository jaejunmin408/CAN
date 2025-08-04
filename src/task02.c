#include "cmsis_os2.h"
#include "main.h"
#include "startDebug.h"
#include <stm32f7xx_hal_iwdg.h>


extern osMessageQueueId_t debugQueue;
extern IWDG_HandleTypeDef hiwdg;

osThreadId_t Task02Handle;
const osThreadAttr_t Task02_attributes = {
  .name = "Task02",
  .stack_size = 1024 * 5,
  .priority = (osPriority_t) osPriorityLow2,
};

void task02_thread(void *argument)
{
  (void)argument;
 
  for(;;)
  {
    
    DEBUG("Task2 Test!!!\n");
    HAL_IWDG_Refresh(&hiwdg);
    osDelay(1000);
  }
}

void task02_init(void *argument)
{
    (void)argument;

    Task02Handle = osThreadNew(task02_thread, NULL, &Task02_attributes);

}