#include <stdint.h>
#include <string.h>

#include "stm32f7xx_hal.h"
#include "stm32f765xx.h"
#include "can.h"
#include "can_user.h"
#include "hardware.h"
#include "cmsis_os2.h"
#include "main.h"
#include "lwip/tcpip.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "ethernetif.h"
#include "lwip/dhcp.h"
#include "lwip/apps/httpd.h"
#include "startDebug.h"
#include "udp_total.h"
#include "task03.h"
#include "task04.h"

// identifier is needed by PEAKFlash.exe -> do not delete
const char Ident[] __attribute__ ((used)) = { "PCAN-Router_Pro_FD"};


// threads/timer
static osTimerId_t  thread_milli_timer_id;

IWDG_HandleTypeDef hiwdg;

//! @brief      a timer based on a high prio thread
static void thread_milli_timer ( void *argument)
{
	HAL_IncTick();
}


//! @brief      primary thread
static void  thread_main ( void  *argument)
{
	// start the timer (RTOS based high prio thread)
	osTimerStart ( thread_milli_timer_id, DLY_MS(1));	
	
	HW_ENA_ETH_PHY;
  	osDelay ( DLY_MS(75));	// for 100Base-TX

	startDebug(NULL);
	printf("START FW\n");
	
	UDP_task(NULL);
	task03_init(NULL);
	task04_init(NULL);
	
	while (1)
	{
		osThreadSuspend(osThreadGetId());
	}
}





//! @brief      entry point
int  main ( void)
{
	//SCB_DisableDCache();

	//initialization
	HW_Init();
	MX_IWDG_Init();
	
	HAL_IWDG_Refresh(&hiwdg);

	// initialize kernel
	osKernelInitialize();
	
	// create primary thread
	{
		osThreadAttr_t  attr;
		memset ( &attr, 0, sizeof ( attr));	
		attr.stack_size = 4 * 1024;
		osThreadId_t tid = osThreadNew ( thread_main, NULL, &attr);
	}
	
	// create a timer
	thread_milli_timer_id = osTimerNew ( thread_milli_timer, osTimerPeriodic, NULL, NULL);
	
	// start thread execution
	osKernelStart();
	
	// should not reach this point !
	while(1)
	 {
		
	 }
}


