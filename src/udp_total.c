/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include "udp_total.h"

#include "cmsis_os2.h"
#include "reset.h"
#include "ARMCM7_DP.h"

/* Private typedef -----------------------------------------------------------*/
#define UDP_SERVER_PORT    8080   /* define the UDP local connection port */
#define UDP_CLIENT_PORT    7   /* define the UDP remote connection port */
#define UDP_SYSTEM_PORT    10 



/* Private pv -----------------------------------------------------------*/
extern osMessageQueueId_t debugQueue;

u8_t   data[100];
struct udp_pcb *upcb;
struct udp_pcb *upcb_sys;
struct udp_pcb *udp_server_pcb;

const char *JUMP_BOOT_STR = "SOFT RESET SSSAAA";
const char *JUMP_FW_STR = "RESET SSSAAA";

osThreadId_t UDPHandle;                       //[user custom] RTOS memory setting + osPriority
const osThreadAttr_t UDP_attributes = {
  .name = "udp_send", 
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityNormal,
};


/* Private functions ---------------------------------------------------------*/
void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
  
  if (p != NULL)
  {
    
     char buffer[12] = {0};                   //딱 부트로더 부르는 명령어의 크기로만 설정해도 되지 않을까
     memcpy(buffer, p->payload, p->len);
     buffer[p->len] = '\0'; // null terminate
     if(strcmp(buffer,JUMP_BOOT_STR) == 0)
     {
      printf("[FW]  JUMP BOOT(soft RESET)\n");
      osDelay(1000);
      write_boot_flag_soft_reset();
      NVIC_SystemReset();
     }
     else if(strcmp(buffer,JUMP_FW_STR ) == 0)
     {
    	 printf("[FW]  JUMP FW(RESET)\n");
       osDelay(1000);
       clear_boot_flag();
    	 NVIC_SystemReset();
     }
     pbuf_free(p);
  }
}



void udp_total_connect(void)
{
  ip_addr_t DestIPaddr;
  err_t err;
  
  /* Create a new UDP control block  */
upcb = udp_new();
  IP4_ADDR( &DestIPaddr, 192, 168, 20, 69);
  err= udp_connect(upcb, &DestIPaddr, UDP_CLIENT_PORT);

  upcb_sys = udp_new();
  IP4_ADDR( &DestIPaddr, 192, 168, 20, 69);
  err= udp_connect(upcb_sys, &DestIPaddr, UDP_SYSTEM_PORT);

  udp_server_pcb = udp_new();
  if (!udp_server_pcb)
  {
    udp_client_send("Can not create pcb1 \n");
  }
  udp_bind(udp_server_pcb, IP_ADDR_ANY, UDP_SERVER_PORT); // 수신 포트
  udp_recv(udp_server_pcb, udp_receive_callback, NULL);
}

void udp_echoclient_send(void)
{
  struct pbuf *p;
  char receiveValue[512];


  osMessageQueueGet(debugQueue, &receiveValue, NULL, osWaitForever);

  sprintf((char*)data, "%s", receiveValue); 
  
  /* allocate pbuf from pool*/
  p = pbuf_alloc(PBUF_TRANSPORT,strlen((char*)data), PBUF_POOL);
  
  if (p != NULL)
  {
    /* copy data to pbuf */
    pbuf_take(p, (char*)data, strlen((char*)data));
    
    /* send udp data */
    udp_send(upcb, p); 
    
    /* free pbuf */
    pbuf_free(p);
  }
}


void udp_client_send(const char *msg)
{
    if (!upcb_sys) {
        printf("[CHECK] UDP PCB not initialized\n");
        return;
    }

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, strlen(msg), PBUF_POOL);
    if (!p) {
        printf("[CHECK] pbuf_alloc failed\n");
        return;
    }

    pbuf_take(p, msg, strlen(msg));

    err_t err = udp_send(upcb_sys, p);
    if (err != ERR_OK) {
        printf("[CHECK] udp_send failed: %d\n", err);
    }

    pbuf_free(p);
}

void UDP_thread(void *argument)
{
  (void)argument;
  for(;;)
  {
    udp_echoclient_send();
    osDelay(1);                      //[user custom] : osDelay setting , now we send UDP message in 1ms
  }
}

void UDP_task(void *argument)
{
    (void)argument;

    UDPHandle = osThreadNew(UDP_thread, NULL, &UDP_attributes);
    if(UDPHandle == NULL)
    {
      printf("[CHECK] task creation failed!, not enough total heap memory\n");
    }
}
