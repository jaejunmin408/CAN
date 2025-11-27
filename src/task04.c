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
extern osMessageQueueId_t canmessageQueue;
extern IWDG_HandleTypeDef hiwdg;


osThreadId_t Task04Handle;
const osThreadAttr_t Task04_attributes = {
  .name = "Task04",
  .stack_size = 1024 * 5,
  .priority = (osPriority_t) osPriorityLow1,
};

void printCanMsg(const CANRxMsgDMA_t* msg)
{
    // CAN BUS 번호
    printf("%7d | ", msg->hBus);

    // CAN FD ID
    printf("0x%08X | ", msg->id);

    // DLC
    printf("%3d | ", msg->dlc);

    // DATA
    for (int i = 0; i < msg->dlc; i++) {
        printf("%02X ", msg->data8[i]);
    }

    printf("\n");
}


void task04_thread(void *argument)
{
  (void)argument;

  for(;;)
  {

    HAL_IWDG_Refresh(&hiwdg);

    static CANRxMsgDMA_t  RxMsg  __attribute__((section(".bss.dtcm")));
    		if ( CAN_UserRead ( CAN_BUSX, &RxMsg) == CAN_ERR_OK)
		{
			switch ( RxMsg.hBus)
			{
				char buf[512]; 
				case CAN_BUS1:
					
					LED_toggleCAN1 ^= 1;

					if ( LED_toggleCAN1)
					{
						HW_SetLED ( HW_LED_CAN1, HW_LED_ORANGE);
					}
					
					else
					{
						HW_SetLED ( HW_LED_CAN1, HW_LED_GREEN);
					}

					DEBUG("CAN1 \n");
					DEBUG("ID : 0x%03X,  DLC : %d\n", RxMsg.id, RxMsg.dlc);
				
					
					snprintf(buf, sizeof(buf), "");

					for (int i = 0; i < RxMsg.dlc; i++)
					{
						snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " 0x%02X", RxMsg.data8[i]);
					}
					DEBUG("%s\n", buf);

				break;

				case CAN_BUS2:
					
					LED_toggleCAN2 ^= 1;

					if ( LED_toggleCAN2)
					{
						HW_SetLED ( HW_LED_CAN2, HW_LED_ORANGE);
					}
					
					else
					{
						HW_SetLED ( HW_LED_CAN2, HW_LED_GREEN);
					}

					// printf("CAN2 \n");
					// printf("ID : 0x%03X,  DLC : %d\n", RxMsg.id, RxMsg.dlc);
				
				
					// snprintf(buf, sizeof(buf), "");

					// for (int i = 0; i < RxMsg.dlc; i++)
					// {
					// 	snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " 0x%02X", RxMsg.data8[i]);
					// }
					// printf("%s\n", buf);

				break;

				case CAN_BUS3:
					
					LED_toggleCAN3 ^= 1;

					if ( LED_toggleCAN3)
					{
						HW_SetLED ( HW_LED_CAN3, HW_LED_ORANGE);
					}
					
					else
					{
						HW_SetLED ( HW_LED_CAN3, HW_LED_GREEN);
					}

					osMessageQueuePut(canmessageQueue, &RxMsg, 0, 0);
			
					 
					// DEBUG("CAN3 \n");
					// DEBUG("ID : 0x%03X,  DLC : %d\n", RxMsg.id, RxMsg.dlc);
				
				
					// snprintf(buf, sizeof(buf), "");

					// for (int i = 0; i < RxMsg.dlc; i++)
					// {
					// 	snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " 0x%02X", RxMsg.data8[i]);
					// }
					// DEBUG("%s\n", buf);

				break;


				case CAN_BUS4:
					
					LED_toggleCAN4 ^= 1;

					printf("TEST!!!\n");

					if ( LED_toggleCAN4)
					{
						HW_SetLED ( HW_LED_CAN4, HW_LED_ORANGE);
					}
					
					else
					{
						HW_SetLED ( HW_LED_CAN4, HW_LED_GREEN);
					}
					printf("TEST!!!\n");
					// DEBUG("CAN4 \n");
					// DEBUG("ID : 0x%03X,  DLC : %d\n", RxMsg.id, RxMsg.dlc);
				
		
					// snprintf(buf, sizeof(buf), "");

					// for (int i = 0; i < RxMsg.dlc; i++)
					// {
					// 	snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " 0x%02X", RxMsg.data8[i]);
					// }
					// DEBUG("%s\n", buf);

				break;


				case CAN_BUS5:
					//여기에 꽂아서 테스트 진행
					//CAN FD extended 형식에 속도도 맞는듯 싶은데..?
					//pc ip: 192.168.20.69 , router ip : 192.168.20.72 
					//pc로 받는 port(pc 서버 port) : 6010(시스템 메세지), 5010(can 메세지) -> server는 이 2개로 키고
					//roouter가 받는 port : 8080 -> client는 192.168.20.72 8080으로 접속
					
					
					LED_toggleCAN5 ^= 1;

					if ( LED_toggleCAN5)
					{
						HW_SetLED ( HW_LED_CAN5, HW_LED_ORANGE);
					}
					
					else
					{
						HW_SetLED ( HW_LED_CAN5, HW_LED_GREEN);
					}


					DEBUG("CAN5 \n");
					DEBUG("ID : 0x%08X, DLC : %d\n", RxMsg.id, RxMsg.dlc);
				 
					snprintf(buf, sizeof(buf), "");

					for (int i = 0; i < RxMsg.dlc; i++)
					{
						snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " 0x%02X", RxMsg.data8[i]);
					}
					DEBUG("%s\n", buf);

				break;


				case CAN_BUS6:
					
					LED_toggleCAN6 ^= 1;

					if ( LED_toggleCAN6)
					{
						HW_SetLED ( HW_LED_CAN6, HW_LED_ORANGE);
					}
					
					else
					{
						HW_SetLED ( HW_LED_CAN6, HW_LED_GREEN);
					}

					DEBUG("CAN6 \n");
					DEBUG("ID : 0x%03X,  DLC : %d\n", RxMsg.id, RxMsg.dlc);
				
			
					snprintf(buf, sizeof(buf), "");

					for (int i = 0; i < RxMsg.dlc; i++)
					{
						snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " 0x%02X", RxMsg.data8[i]);
					}
					DEBUG("%s\n", buf);

				break;

				default:
					DEBUG("UNKNOWN CAN: hBus=%d, ID=0x%03X\n", RxMsg.hBus, RxMsg.id);
				break;

			}
		}
		// uint32_t now = HAL_GetTick();
		// if (now - lastGreetingTick_CAN2 > 2000) HW_SetLED(HW_LED_CAN2, HW_LED_GREEN);
		osDelay (DLY_MS(1));	
  }
}

void task04_init(void *argument)
{
    (void)argument;
	DEBUG("TASK4 START\n");

    Task04Handle = osThreadNew(task04_thread, NULL, &Task04_attributes);

}