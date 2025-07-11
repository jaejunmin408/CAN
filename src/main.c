
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

#include "reset.h"
#include "udp_total.h"
#include "stm32f7xx_hal_rcc_ex.h"
#include "tftpserver.h"
#include "firmware_tag.h"
#include "backup.h"


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

uint8_t check = 1;

CRC_HandleTypeDef hcrc;

typedef  void (*pFunction)(void);
#define TAG_ADDRESS  ((FirmwareTag*) USER_FLASH_FIRST_PAGE_ADDRESS)
#define BACKUP_ADDRESS  ((FirmwareTag*) BACKUP_FLASH_FIRST_PAGE_ADDRESS)
pFunction JumpToApplication;
uint32_t JumpAddress;

//! @brief      a timer based on a high prio thread
static void thread_milli_timer ( void *argument)
{
	HAL_IncTick();
}

static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  __HAL_RCC_CRC_CLK_ENABLE();
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  HAL_CRC_Init(&hcrc);
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}



//! @brief      primary thread
static void  thread_main ( void  *argument)
{
	// start the timer (RTOS based high prio thread)
	osTimerStart ( thread_milli_timer_id, DLY_MS(1));
	
	// init CAN
	CAN_UserInit();

	MX_CRC_Init();

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

	udp_total_connect();
	udp_client_send("Hello World!\n");

	IAP_tftpd_init();
	osDelay(1000);

    // /* Initialize
	// // start dhcp
	// dhcp_start ( &gnetif);

	while ( 1)
	{
		//osThreadSuspend(osThreadGetId());
		if ( is_soft_reset() != 0 )
		{
			if(check)
        	{
				udp_client_send("[BOOT]  START BOOT...\n");
          		check = 0;
        	}
		}
		osDelay(1);

		//osThreadSuspend(osThreadGetId());
	}

}






//! @brief      entry point
int  main ( void)
{
	// disable DCache. To use DCache you have to place ethernet buffers
	// into DTCM memory cause CPU cache is not visible to ETH DMA controller.

	//SCB_DisableDCache(); 	//check this!!!

	// finalize initialization
	
	HW_Init();

	if ( is_soft_reset() == 0 )
	{
		uint32_t isr_vector_addr = USER_FLASH_FIRST_PAGE_ADDRESS + 0x400;
		volatile uint32_t app_sp_value = *(__IO uint32_t*) isr_vector_addr;
		if (app_sp_value >= 0x20000000 && app_sp_value <= 0x20080000)
		{

				// HAL_RCC_DeInit();
				// HAL_DeInit();
				// __disable_irq();
				// SysTick->CTRL = 0;
				// SysTick->LOAD = 0;
				// SysTick->VAL  = 0;
				// for (uint32_t i = 0; i < 8; i++)
				// {
				// 	NVIC->ICER[i] = 0xFFFFFFFF;
				// 	NVIC->ICPR[i] = 0xFFFFFFFF;
				// }
				// SCB_DisableICache();
				// SCB_DisableDCache();

				JumpAddress = *(__IO uint32_t*) (isr_vector_addr + 4);
				JumpToApplication = (pFunction) JumpAddress;


				/* Initialize user application's Stack Pointer */
				__set_MSP(*(__IO uint32_t*) isr_vector_addr);
				SCB->VTOR = USER_FLASH_FIRST_PAGE_ADDRESS;   
				JumpToApplication();
		}
		else
		{
			write_boot_flag_soft_reset();
			//udp_client_send("[BOOT]  No FW in flash, back to BOOT\n");
		  	NVIC_SystemReset();
		}
	}

	// initialize kernel
	osKernelInitialize();
	
	// create primary thread
	{
		osThreadAttr_t  attr;


		memset ( &attr, 0, sizeof ( attr));	//0

		attr.stack_size = 4*1024;

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


