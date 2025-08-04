#include "cmsis_os2.h"
#include "main.h"
#include "startDebug.h"
#include "can.h"
#include "can_user.h"
#include "hardware.h"
#include <stm32f7xx_hal_iwdg.h>
#include "udp_total.h"
#include "task04.h"



extern osMessageQueueId_t debugQueue;
extern IWDG_HandleTypeDef hiwdg;


osThreadId_t Task04Handle;
const osThreadAttr_t Task04_attributes = {
  .name = "Task04",
  .stack_size = 1024 * 5,
  .priority = (osPriority_t) osPriorityLow1,
};


// static void  main_greeting ( void)
// {
// 	CANTxMsg_t  msg;
	
	
// 	msg.bufftype = CAN_BUFFER_TX_MSG;
// 	msg.dlc      = CAN_LEN8_DLC;
// 	msg.msgtype  = CAN_MSGTYPE_STANDARD;
// 	msg.id       = 0x123;
	
// 	msg.data32[0] = 0x67452301;
// 	msg.data32[1] = 0xEFCDAB89;
	
// 	// patch byte 0 with FPGA version
// 	msg.data8[0] = HW_FPGA_VERSION;
	
// 	// Send message
// 	CAN_Write ( CAN_BUS1, &msg);
// }


void task04_thread(void *argument)
{
  (void)argument;

  // init CAN
	CAN_UserInit();

  // set green LEDs for CANs
	// HW_SetLED ( HW_LED_CAN1, HW_LED_GREEN);
	// HW_SetLED ( HW_LED_CAN2, HW_LED_GREEN);
	// HW_SetLED ( HW_LED_CAN3, HW_LED_GREEN);
	// HW_SetLED ( HW_LED_CAN4, HW_LED_GREEN);
	// HW_SetLED ( HW_LED_CAN5, HW_LED_GREEN);
	// HW_SetLED ( HW_LED_CAN6, HW_LED_GREEN);

  //main_greeting();
  for(;;)
  {
    HAL_IWDG_Refresh(&hiwdg);

    static CANRxMsgDMA_t  RxMsg  __attribute__((section(".bss.dtcm")));
    		if ( CAN_UserRead ( CAN_BUSX, &RxMsg) == CAN_ERR_OK)
		{
			switch ( RxMsg.hBus)
			{
				
				case CAN_BUS3:
					// message received from CAN1
					LED_toggleCAN3 ^= 1;

					if ( LED_toggleCAN3)
					{
						HW_SetLED ( HW_LED_CAN3, HW_LED_ORANGE);
					}
					
					else
					{
						HW_SetLED ( HW_LED_CAN3, HW_LED_GREEN);
					}

					printf("[FW] CAN3 \n");
					printf("[FW]  ID : 0x%03X,  DLC : %d\n", RxMsg.id, RxMsg.dlc);
				
					char buf[128]; 
					snprintf(buf, sizeof(buf), "[FW]");

					for (int i = 0; i < RxMsg.dlc; i++)
					{
						snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " 0x%02X", RxMsg.data8[i]);
					}
					printf("%s\n", buf);


					// // catch ID 123h and convert it from classical CAN to CAN-FD
					// if ( RxMsg.id == 0x123  &&  RxMsg.msgtype == CAN_MSGTYPE_STANDARD)
					// {
					// 	// overwrite msgtype
					// 	RxMsg.msgtype = CAN_MSGTYPE_FDF | CAN_MSGTYPE_BRS;

					// 	// set dlc to 12 data bytes
					// 	RxMsg.dlc = CAN_LEN12_DLC;

					// 	// add some other data
					// 	RxMsg.data8[8] = 0x11;
					// 	RxMsg.data8[9] = 0x22;
					// 	RxMsg.data8[10] = 0x33;
					// 	RxMsg.data8[11] = 0x44;
					// }

 					// // forward message to CAN2
					// CAN_Write ( CAN_BUS2, &RxMsg);
					break;
					default:
					printf("알 수 없는 CAN 수신 버스: hBus=%d, ID=0x%03X\n", RxMsg.hBus, RxMsg.id);
					break;
					
					
				// case CAN_BUS2:
				// 	// message received from CAN2
				// 	LED_toggleCAN2 ^= 1;

				// 	if ( LED_toggleCAN2)
				// 	{
				// 		HW_SetLED ( HW_LED_CAN2, HW_LED_ORANGE);
				// 	}
					
				// 	else
				// 	{
				// 		HW_SetLED ( HW_LED_CAN2, HW_LED_GREEN);
				// 	}
					
                // 	if (RxMsg.id == 0x123 && RxMsg.dlc >= 1 && RxMsg.data8[0] == HW_FPGA_VERSION)
                // 	{
                //    		HW_SetLED(HW_LED_CAN2, HW_LED_ORANGE);
                //     	lastGreetingTick_CAN2 = HAL_GetTick();
                // 	}

				// 	// CAN1 will be classic CAN only. If msgs will have more than
				// 	// eight bytes you have to fragment them manually.
				// 	RxMsg.msgtype &= ~( CAN_MSGTYPE_FDF | CAN_MSGTYPE_BRS);

				// 	printf("[FW]  First byte: 0x%02X\n", RxMsg.data8[8]);
				// 	// forward message to CAN1
				// 	CAN_Write ( CAN_BUS1, &RxMsg);
				// 	break;
					
					
				// case CAN_BUS3:
				// 	// message received from CAN3
				// 	LED_toggleCAN3 ^= 1;

				// 	if ( LED_toggleCAN3)
				// 	{
				// 		HW_SetLED ( HW_LED_CAN3, HW_LED_ORANGE);
				// 	}
					
				// 	else
				// 	{
				// 		HW_SetLED ( HW_LED_CAN3, HW_LED_GREEN);
				// 	}
					
				// 	// forward message to CAN4
				// 	CAN_Write ( CAN_BUS4, &RxMsg);
				// 	break;
					
					
				// case CAN_BUS4:
				// 	// message received from CAN4
				// 	LED_toggleCAN4 ^= 1;

				// 	if ( LED_toggleCAN4)
				// 	{
				// 		HW_SetLED ( HW_LED_CAN4, HW_LED_ORANGE);
				// 	}
					
				// 	else
				// 	{
				// 		HW_SetLED ( HW_LED_CAN4, HW_LED_GREEN);
				// 	}
					
				// 	// forward message to CAN3
				// 	CAN_Write ( CAN_BUS3, &RxMsg);
				// 	break;
					
					
				// case CAN_BUS5:
				// 	// message received from CAN5
				// 	LED_toggleCAN5 ^= 1;

				// 	if ( LED_toggleCAN5)
				// 	{
				// 		HW_SetLED ( HW_LED_CAN5, HW_LED_ORANGE);
				// 	}
					
				// 	else
				// 	{
				// 		HW_SetLED ( HW_LED_CAN5, HW_LED_GREEN);
				// 	}
					
				// 	// forward message to CAN6
				// 	CAN_Write ( CAN_BUS6, &RxMsg);
				// 	break;
					
					
				// case CAN_BUS6:
				// 	// message received from CAN6
				// 	LED_toggleCAN6 ^= 1;

				// 	if ( LED_toggleCAN6)
				// 	{
				// 		HW_SetLED ( HW_LED_CAN6, HW_LED_ORANGE);
				// 	}
					
				// 	else
				// 	{
				// 		HW_SetLED ( HW_LED_CAN6, HW_LED_GREEN);
				// 	}
					
				// 	// forward message to CAN5
				// 	CAN_Write ( CAN_BUS5, &RxMsg);
				// 	break;
			}
		}

		// uint32_t now = HAL_GetTick();
		// if (now - lastGreetingTick_CAN2 > 2000) HW_SetLED(HW_LED_CAN2, HW_LED_GREEN);

		osDelay (DLY_MS(1));	//DLY_MS(1)
  }
}

void task04_init(void *argument)
{
    (void)argument;

    Task04Handle = osThreadNew(task04_thread, NULL, &Task04_attributes);

}