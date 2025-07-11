#include "startDebug.h"

#define INTERFACE_THREAD_STACK_SIZE ( 1024 )

osMessageQueueId_t debugQueue;      //for Debug Queue
osThreadAttr_t attributes;

/*
  open udp_connect
  create Queue
*/
void startDebug(void *argument)
{
    udp_total_connect();
    printf("[FW]  START FW\n");
    //osDelay(2000);
    debugQueue = osMessageQueueNew(QUEUE_SIZE, ITEM_SIZE, NULL);
}




/*
  if printf("[CHECK] ~~") : UART print
  else printf("~~") : put debugQueue
*/
int _write(int file, char *ptr, int len)
{
    (void)file;
  
  if (strstr(ptr, "[CHECK]" ) != NULL)
  {
     
  }
  else if(strstr(ptr, "[FW]" ) != NULL)
  {
    char queue_buf[(sizeof(char) * 512)] = {0};
    strncpy(queue_buf, ptr, len);
    queue_buf[len] = '\0';
    udp_client_send(queue_buf);
  }
  else
  {
    char queue_buf[(sizeof(char) * 512)] = {0};
    strncpy(queue_buf, ptr, len);
    queue_buf[len] = '\0';
    osMessageQueuePut(debugQueue, queue_buf, 0, 0);
  }
  return len;
}

// //check free Heap size(check your option configTOTAL_HEAP_size)
// void checkHeapMemory(void *argument)
// {
//    printf("[CHECK] free Heap size: %d \n",xPortGetMinimumEverFreeHeapSize());
// }