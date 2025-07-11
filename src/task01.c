#include "cmsis_os2.h"
#include "main.h"
#include "startDebug.h"


extern osMessageQueueId_t debugQueue; 
extern IWDG_HandleTypeDef hiwdg;        

osThreadId_t Task01Handle;
const osThreadAttr_t Task01_attributes = {
  .name = "Task01",
  .stack_size = 1024 * 5,
  .priority = (osPriority_t) osPriorityLow1,
};

void task01_thread(void *argument)
{
  (void)argument;
 
  for(;;)
  {
   
    DEBUG("Task1 Test!!!\n");
    HAL_IWDG_Refresh(&hiwdg);
    osDelay(1000);
  }
}

void task01_init(void *argument)
{
    (void)argument;

    Task01Handle = osThreadNew(task01_thread, NULL, &Task01_attributes);

}