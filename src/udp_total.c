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
#include <stm32f7xx_hal_iwdg.h>
#include "lwip/etharp.h"

#include <lwip/sockets.h>
#include <can.h>
#include <can_user.h>


/* Private typedef -----------------------------------------------------------*/
#define UDP_SERVER_PORT    8080   /* define the UDP local connection port */
#define UDP_CAN_PORT       5010
#define UDP_CLIENT_PORT    6010   /* define the UDP remote connection port */



/* Private pv -----------------------------------------------------------*/
extern osMessageQueueId_t debugQueue;
extern osMessageQueueId_t canmessageQueue;
extern IWDG_HandleTypeDef hiwdg;
extern struct netif gnetif;

u8_t   data[100];
struct udp_pcb *udp_client_pcb;
struct udp_pcb *udp_server_pcb;
struct udp_pcb *udp_can_pcb;

const char *JUMP_BOOT_STR = "SOFT RESET SSSAAA";
const char *JUMP_FW_STR = "RESET SSSAAA";



osThreadId_t UDPHandle;                       //[user custom] RTOS memory setting + osPriority
const osThreadAttr_t UDP_attributes = {
  .name = "udp_send", 
  .stack_size = 2048, 
  .priority = (osPriority_t) osPriorityBelowNormal,
};

typedef struct
{
    uint32_t id;          // CAN Identifier
    uint8_t  dlc;         // Data Length Code (0~8 또는 64)
    uint8_t  is_fd;       // CAN FD 여부 (0=CAN, 1=CAN FD)
    uint8_t  reserved[2]; // 정렬 맞추기용 padding
    uint8_t  data[64];    // CAN 데이터
} UDPCANMsg_t;



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
  udp_client_pcb = udp_new();
  IP4_ADDR( &DestIPaddr, 192, 168, 20, 69);
  err= udp_connect(udp_client_pcb, &DestIPaddr, UDP_CLIENT_PORT);

  udp_can_pcb = udp_new();
  IP4_ADDR( &DestIPaddr, 192, 168, 20, 69);
  err= udp_connect(udp_can_pcb, &DestIPaddr, UDP_CAN_PORT);

  udp_server_pcb = udp_new();
  if (!udp_server_pcb)
  {
    printf("[FW] Can not create pcb1 \n");
  }
  udp_bind(udp_server_pcb, IP_ADDR_ANY, UDP_SERVER_PORT); // 수신 포트
  udp_recv(udp_server_pcb, udp_receive_callback, NULL);
}

//linux에서 그냥 출력 버전
void udp_can_send(void)
{
  struct pbuf *p;
  osStatus_t status;
  CANRxMsgDMA_t rxMsg;   // 큐에서 꺼낼 CAN 메시지
  
  status = osMessageQueueGet(canmessageQueue, &rxMsg, NULL, 0);
  uint16_t msg_size = sizeof(CANRxMsgDMA_t);    //보내는 메세지 크기를 고정 -> 나중에 간략한 버전으로 업데이트 할것

  // if (msg_size == 0 || msg_size > sizeof(CANRxMsgDMA_t)) {
  //       msg_size = sizeof(CANRxMsgDMA_t);
  // }

    // 큐에 메시지가 있을 때만 전송
    if (status == osOK)
    {
        p = pbuf_alloc(PBUF_TRANSPORT, msg_size, PBUF_POOL);
        if (p != NULL)
        {
            // 구조체 메모리 그대로 복사
            pbuf_take(p, &rxMsg, msg_size);

            // UDP 전송
            udp_send(udp_can_pcb, p);

            // pbuf 해제
            pbuf_free(p);
        }
    }
    // 큐가 비어있으면 아무 동작도 하지 않음
}

//savvy can 버전
void udp_can_QT_send(void)
{
  struct pbuf *p;
    osStatus_t status;
    CANRxMsgDMA_t rxMsg;
    UDPCANMsg_t udpMsg;

    // 1️⃣ CAN 메시지 큐에서 수신
    status = osMessageQueueGet(canmessageQueue, &rxMsg, NULL, 0);
    if (status != osOK)
        return;

    // 2️⃣ 필요한 필드만 추출
    udpMsg.id  = rxMsg.id;
    udpMsg.dlc = rxMsg.dlc;
    udpMsg.is_fd = (rxMsg.msgtype & 0x01) ? 1 : 0; // 필요 시 FD 여부 판단

    // DLC 길이만큼 데이터 복사 (64바이트 넘으면 64로 제한)
    uint8_t len = (udpMsg.dlc > 64) ? 64 : udpMsg.dlc;
    memcpy(udpMsg.data, rxMsg.data8, len);

    // 나머지 데이터는 0으로 초기화
    if (len < 64)
        memset(&udpMsg.data[len], 0, 64 - len);

    // 3️⃣ UDP로 전송
    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(UDPCANMsg_t), PBUF_POOL);
    if (p != NULL)
    {
        pbuf_take(p, &udpMsg, sizeof(UDPCANMsg_t));
        udp_send(udp_can_pcb, p);
        pbuf_free(p);
    }
}

void udp_echoclient_send(void)
{
  struct pbuf *p;
  char receiveValue[512];
  osStatus_t status;
  
  status = osMessageQueueGet(debugQueue, &receiveValue, NULL, 0);


  if( status == osOK){
    sprintf((char*)data, "%s", receiveValue); 
    
    /* allocate pbuf from pool*/
    p = pbuf_alloc(PBUF_TRANSPORT,strlen((char*)data), PBUF_POOL);
    
    if (p != NULL)
    {
      /* copy data to pbuf */
      pbuf_take(p, (char*)data, strlen((char*)data));
      
      /* send udp data */
      udp_send(udp_client_pcb, p); 
      
      /* free pbuf */
      pbuf_free(p);
    }
  }

  else if( status == osErrorResource){
    
  }
}

void UDP_thread(void *argument)
{
  (void)argument;
  for(;;)
  {
    udp_echoclient_send();
    udp_can_send();
    //udp_can_QT_send();
    HAL_IWDG_Refresh(&hiwdg);
    osDelay(DLY_MS(20));                      //[user custom] : osDelay setting , now we send UDP message in 1ms
  }
}


void UDP_task(void *argument)
{
    (void)argument;

    UDPHandle = osThreadNew(UDP_thread, NULL, &UDP_attributes);
    if(UDPHandle == NULL)
    {
      
    }
}