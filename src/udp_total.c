/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include "udp_total.h"

#include "cmsis_os2.h"
#include "ARMCM7_DP.h"

/* Private typedef -----------------------------------------------------------*/
#define UDP_SERVER_PORT    8080   /* define the UDP local connection port */
#define UDP_CLIENT_PORT    6010   /* define the UDP remote connection port */


/* Private pv -----------------------------------------------------------*/

u8_t   data[100];
struct udp_pcb *upcb_boot;
struct udp_pcb *udp_server_pcb_boot;

const char *JUMP_BOOT_STR = "SOFT RESET SSSAAA";
const char *JUMP_FW_STR = "RESET SSSAAA";


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
      udp_client_send("[BOOT]  JUMP BOOT(soft RESET)\n");
      write_boot_flag_soft_reset();
      NVIC_SystemReset();
     }
     else if(strcmp(buffer,JUMP_FW_STR ) == 0)
     {
    	 udp_client_send("[BOOT]  JUMP FW(RESET)\n");
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
  upcb_boot = udp_new();
  IP4_ADDR( &DestIPaddr, 192, 168, 20, 69);
  err= udp_connect(upcb_boot, &DestIPaddr, UDP_CLIENT_PORT);

  udp_server_pcb_boot = udp_new();
  udp_bind(udp_server_pcb_boot, IP_ADDR_ANY, UDP_SERVER_PORT); // 수신 포트
  udp_recv(udp_server_pcb_boot, udp_receive_callback, NULL);
}

void udp_client_send(const char *msg)
{
    if (!upcb_boot) {
        printf("[BOOT]  UDP PCB not initialized\n");
        return;
    }

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, strlen(msg), PBUF_POOL);
    if (!p) {
        printf("[BOOT]  pbuf_alloc failed\n");
        return;
    }

    pbuf_take(p, msg, strlen(msg));

    err_t err = udp_send(upcb_boot, p);
    if (err != ERR_OK) {
        printf("[BOOT]  udp_send failed: %d\n", err);
    }
    osDelay(1);
    pbuf_free(p);
}
