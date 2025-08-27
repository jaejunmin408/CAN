#include "startDebug.h"
#include <stdio.h>
#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include "udp_total.h"
#include "ethernetif.h"
#include "cmsis_os2.h"
#include "lwip/tcpip.h"
#include "stm32f765xx.h"
#include <can_user.h>


#define INTERFACE_THREAD_STACK_SIZE ( 1024 )

osMessageQueueId_t debugQueue;      //for Debug Queue
osMessageQueueId_t canmessageQueue;


osThreadAttr_t attributes;
extern IWDG_HandleTypeDef hiwdg;

/*
  open udp_connect
  create Queue
*/
void startDebug(void *argument)
{
    LWIP_init(NULL);
    udp_total_connect();
    debugQueue = osMessageQueueNew(QUEUE_SIZE, ITEM_SIZE, NULL);
    canmessageQueue = osMessageQueueNew(10, sizeof(CANRxMsgDMA_t), NULL);

    osDelay(DLY_MS(1000));
    HAL_IWDG_Refresh(&hiwdg);
}




/*
  if printf("[CHECK] ~~") : UART print
  else printf("~~") : put debugQueue
*/
int _write(int file, char *ptr, int len)
{
    (void)file;
  
  if(strstr(ptr, "[FW]" ) != NULL)
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

void MX_IWDG_Init(void)
{
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Window = 4095;
  hiwdg.Init.Reload = 4095;
}


//for lwip
struct netif gnetif;
ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;
uint8_t IP_ADDRESS[4];
uint8_t NETMASK_ADDRESS[4];
uint8_t GATEWAY_ADDRESS[4];

void LWIP_init(void *argument){

	// init lwip stack
	tcpip_init( NULL, NULL );

  // init ip address, requested later by dhcp
  IP_ADDRESS[0] = 192;
  IP_ADDRESS[1] = 168;
  IP_ADDRESS[2] = 20;
  IP_ADDRESS[3] = 72;
  NETMASK_ADDRESS[0] = 255;
  NETMASK_ADDRESS[1] = 255;
  NETMASK_ADDRESS[2] = 255;
  NETMASK_ADDRESS[3] = 0;
  GATEWAY_ADDRESS[0] = 192;
  GATEWAY_ADDRESS[1] = 168;
  GATEWAY_ADDRESS[2] = 20;
  GATEWAY_ADDRESS[3] = 1;

  IP4_ADDR(&ipaddr, IP_ADDRESS[0], IP_ADDRESS[1], IP_ADDRESS[2], IP_ADDRESS[3]);
  IP4_ADDR(&netmask, NETMASK_ADDRESS[0], NETMASK_ADDRESS[1] , NETMASK_ADDRESS[2], NETMASK_ADDRESS[3]);
  IP4_ADDR(&gw, GATEWAY_ADDRESS[0], GATEWAY_ADDRESS[1], GATEWAY_ADDRESS[2], GATEWAY_ADDRESS[3]);

	// add the network interface
	netif_add ( &gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);


	// set default netif
	netif_set_default ( &gnetif);

	if ( netif_is_link_up ( &gnetif))
	{
		/* When the netif is fully configured this function must be called */
		netif_set_up ( &gnetif);
	}

	else
	{
		/* When the netif link is down this function must be called */
		netif_set_down ( &gnetif);
	}
}

// //check free Heap size(check your option configTOTAL_HEAP_size)
// void checkHeapMemory(void *argument)
// {
//    printf("[CHECK] free Heap size: %d \n",xPortGetMinimumEverFreeHeapSize());
// }