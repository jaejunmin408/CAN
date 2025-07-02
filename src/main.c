
#include <stdint.h>
#include <string.h>
#include "can.h"
#include "can_user.h"
#include "hardware.h"
#include "cmsis_os2.h"
#include "stm32f7xx_hal.h"
#include "main.h"

#include "lwip/tcpip.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "ethernetif.h"
#include "lwip/dhcp.h"
#include "lwip/apps/httpd.h"

#include "startDebug.h"
#include "udp_total.h"


// abstract:
// this example shows a http server based on the code from
// the LwIP stack. For more information on lwIP see
// https://savannah.nongnu.org/projects/lwip/






// identifier is needed by PEAKFlash.exe -> do not delete
const char Ident[] __attribute__ ((used)) = { "PCAN-Router_Pro_FD"};


// threads/timer
static osTimerId_t  thread_milli_timer_id;


// for lwip
struct netif gnetif;
ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;
uint8_t IP_ADDRESS[4];
uint8_t NETMASK_ADDRESS[4];
uint8_t GATEWAY_ADDRESS[4];
static uint8_t LED_toggleCAN1;
static uint8_t LED_toggleCAN2;
static uint8_t LED_toggleCAN3;
static uint8_t LED_toggleCAN4;
static uint8_t LED_toggleCAN5;
static uint8_t LED_toggleCAN6;

uint32_t lastGreetingTick_CAN2 = 0;



//! @brief      a timer based on a high prio thread
static void thread_milli_timer ( void *argument)
{
	HAL_IncTick();
}

static void  main_greeting ( void)
{
	CANTxMsg_t  msg;
	
	
	msg.bufftype = CAN_BUFFER_TX_MSG;
	msg.dlc      = CAN_LEN8_DLC;
	msg.msgtype  = CAN_MSGTYPE_STANDARD;
	msg.id       = 0x123;
	
	msg.data32[0] = 0x67452301;
	msg.data32[1] = 0xEFCDAB89;
	
	// patch byte 0 with FPGA version
	msg.data8[0] = HW_FPGA_VERSION;
	
	// Send message
	CAN_Write ( CAN_BUS1, &msg);
}



//! @brief      primary thread
static void  thread_main ( void  *argument)
{
	// start the timer (RTOS based high prio thread)
	osTimerStart ( thread_milli_timer_id, DLY_MS(1));
	
	// init CAN
	CAN_UserInit();
	
	// set green LEDs for CANs
	HW_SetLED ( HW_LED_CAN1, HW_LED_GREEN);
	HW_SetLED ( HW_LED_CAN2, HW_LED_GREEN);
	HW_SetLED ( HW_LED_CAN3, HW_LED_GREEN);
	HW_SetLED ( HW_LED_CAN4, HW_LED_GREEN);
	HW_SetLED ( HW_LED_CAN5, HW_LED_GREEN);
	HW_SetLED ( HW_LED_CAN6, HW_LED_GREEN);


	// put 100Base-T1 PHY into slave mode
	#if _PHY_IS_T1_
	HW_BR_REACH_SLAVE;
	#else
	//HW_BR_REACH_MASTER;
	#endif

	// enable PHY, deassert reset
	HW_ENA_ETH_PHY;
	osDelay ( DLY_MS(75));	// for 100Base-TX
	
	
	// init lwip stack
	tcpip_init( NULL, NULL );


	// init ip address, requested later by dhcp
  	IP_ADDRESS[0] = 192;
  	IP_ADDRESS[1] = 168;
  	IP_ADDRESS[2] = 20;
  	IP_ADDRESS[3] = 70;
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


	// // start dhcp
	// dhcp_start ( &gnetif);
	startDebug(NULL);
	main_greeting();

	task01_init(NULL);
  	task02_init(NULL);
  	UDP_task(NULL);

	while ( 1)
	{
		osThreadSuspend(osThreadGetId());
		// CANTxMsg_t  msg;


		// msg.bufftype = CAN_BUFFER_TX_MSG;
		// msg.dlc      = CAN_LEN4_DLC;
		// msg.msgtype  = CAN_MSGTYPE_STANDARD;
		// msg.id       = 0x456;

		// msg.data32[0] = 0x0;

    // static CANRxMsgDMA_t  RxMsg  __attribute__((section(".bss.dtcm")));
    // 		if ( CAN_UserRead ( CAN_BUSX, &RxMsg) == CAN_ERR_OK)
	// 	{
	// 		switch ( RxMsg.hBus)
	// 		{
	// 			case CAN_BUS1:
	// 				// message received from CAN1
	// 				LED_toggleCAN1 ^= 1;

	// 				if ( LED_toggleCAN1)
	// 				{
	// 					HW_SetLED ( HW_LED_CAN1, HW_LED_ORANGE);
	// 				}
					
	// 				else
	// 				{
	// 					HW_SetLED ( HW_LED_CAN1, HW_LED_GREEN);
	// 				}

	// 				// catch ID 123h and convert it from classical CAN to CAN-FD
	// 				if ( RxMsg.id == 0x123  &&  RxMsg.msgtype == CAN_MSGTYPE_STANDARD)
	// 				{
	// 					// overwrite msgtype
	// 					RxMsg.msgtype = CAN_MSGTYPE_FDF | CAN_MSGTYPE_BRS;

	// 					// set dlc to 12 data bytes
	// 					RxMsg.dlc = CAN_LEN12_DLC;

	// 					// add some other data
	// 					RxMsg.data8[8] = 0x11;
	// 					RxMsg.data8[9] = 0x22;
	// 					RxMsg.data8[10] = 0x33;
	// 					RxMsg.data8[11] = 0x44;
	// 				}

	// 				// forward message to CAN2
	// 				CAN_Write ( CAN_BUS2, &RxMsg);
	// 				break;
					
					
	// 			case CAN_BUS2:
	// 				// message received from CAN2
	// 				LED_toggleCAN2 ^= 1;

	// 				if ( LED_toggleCAN2)
	// 				{
	// 					HW_SetLED ( HW_LED_CAN2, HW_LED_ORANGE);
	// 				}
					
	// 				else
	// 				{
	// 					HW_SetLED ( HW_LED_CAN2, HW_LED_GREEN);
	// 				}
					
    //             	if (RxMsg.id == 0x123 && RxMsg.dlc >= 1 && RxMsg.data8[0] == HW_FPGA_VERSION)
    //             	{
    //                		HW_SetLED(HW_LED_CAN2, HW_LED_ORANGE);
    //                 	lastGreetingTick_CAN2 = HAL_GetTick();
    //             	}

	// 				// CAN1 will be classic CAN only. If msgs will have more than
	// 				// eight bytes you have to fragment them manually.
	// 				RxMsg.msgtype &= ~( CAN_MSGTYPE_FDF | CAN_MSGTYPE_BRS);

	// 				// forward message to CAN1
	// 				CAN_Write ( CAN_BUS1, &RxMsg);
	// 				break;
					
					
	// 			case CAN_BUS3:
	// 				// message received from CAN3
	// 				LED_toggleCAN3 ^= 1;

	// 				if ( LED_toggleCAN3)
	// 				{
	// 					HW_SetLED ( HW_LED_CAN3, HW_LED_ORANGE);
	// 				}
					
	// 				else
	// 				{
	// 					HW_SetLED ( HW_LED_CAN3, HW_LED_GREEN);
	// 				}
					
	// 				// forward message to CAN4
	// 				CAN_Write ( CAN_BUS4, &RxMsg);
	// 				break;
					
					
	// 			case CAN_BUS4:
	// 				// message received from CAN4
	// 				LED_toggleCAN4 ^= 1;

	// 				if ( LED_toggleCAN4)
	// 				{
	// 					HW_SetLED ( HW_LED_CAN4, HW_LED_ORANGE);
	// 				}
					
	// 				else
	// 				{
	// 					HW_SetLED ( HW_LED_CAN4, HW_LED_GREEN);
	// 				}
					
	// 				// forward message to CAN3
	// 				CAN_Write ( CAN_BUS3, &RxMsg);
	// 				break;
					
					
	// 			case CAN_BUS5:
	// 				// message received from CAN5
	// 				LED_toggleCAN5 ^= 1;

	// 				if ( LED_toggleCAN5)
	// 				{
	// 					HW_SetLED ( HW_LED_CAN5, HW_LED_ORANGE);
	// 				}
					
	// 				else
	// 				{
	// 					HW_SetLED ( HW_LED_CAN5, HW_LED_GREEN);
	// 				}
					
	// 				// forward message to CAN6
	// 				CAN_Write ( CAN_BUS6, &RxMsg);
	// 				break;
					
					
	// 			case CAN_BUS6:
	// 				// message received from CAN6
	// 				LED_toggleCAN6 ^= 1;

	// 				if ( LED_toggleCAN6)
	// 				{
	// 					HW_SetLED ( HW_LED_CAN6, HW_LED_ORANGE);
	// 				}
					
	// 				else
	// 				{
	// 					HW_SetLED ( HW_LED_CAN6, HW_LED_GREEN);
	// 				}
					
	// 				// forward message to CAN5
	// 				CAN_Write ( CAN_BUS5, &RxMsg);
	// 				break;
	// 		}
	// 	}

	// 	uint32_t now = HAL_GetTick();
	// 	if (now - lastGreetingTick_CAN2 > 2000) HW_SetLED(HW_LED_CAN2, HW_LED_GREEN);

	// 	osDelay ( DLY_MS(5000));

		// if ( dhcp_supplied_address ( &gnetif))
		// {
		// 	// IPv4 used here
		// 	msg.data32[0] = gnetif.ip_addr.addr;

		// 	// Send message
		// 	CAN_Write ( CAN_BUS1, &msg);
		// 	break;
		// }

		// else
		// {
		// 	// Send message
		// 	CAN_Write ( CAN_BUS1, &msg);
		// }
	}


	// init http server
	// httpd_init();


	// while(1)
	// {	
	// 	osDelay ( DLY_MS(1000));
	// }

}





//! @brief      entry point
int  main ( void)
{
	// disable DCache. To use DCache you have to place ethernet buffers
	// into DTCM memory cause CPU cache is not visible to ETH DMA controller.
	SCB_DisableDCache();

	// finalize initialization
	HW_Init();
	
	// initialize kernel
	osKernelInitialize();
	
	// create primary thread
	{
		osThreadAttr_t  attr;


		memset ( &attr, 0, sizeof ( attr));	//0

		attr.stack_size = 2*1024;

		osThreadNew ( thread_main, NULL, &attr);
	}
	
	// create a timer
	thread_milli_timer_id = osTimerNew ( thread_milli_timer, osTimerPeriodic, NULL, NULL);
	
	// start thread execution
	osKernelStart();
	
	// should not reach this point !
	while(1)
	 {}
}


