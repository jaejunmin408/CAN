#include "cmsis_os2.h"
#include "main.h"
#include "startDebug.h"
#include "can.h"
#include "can_user.h"
#include "hardware.h"
#include <stm32f7xx_hal_iwdg.h>
#include "udp_total.h"
#include "task03.h"



extern osMessageQueueId_t debugQueue;
extern IWDG_HandleTypeDef hiwdg;



osThreadId_t Task03Handle;
const osThreadAttr_t Task03_attributes = {
  .name = "Task03",
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


void task03_thread(void *argument)
{
  (void)argument;

  // init CAN
	CAN_UserInit();

  // set green LEDs for CANs
	HW_SetLED ( HW_LED_CAN1, HW_LED_GREEN);
	HW_SetLED ( HW_LED_CAN2, HW_LED_GREEN);
	HW_SetLED ( HW_LED_CAN3, HW_LED_GREEN);
	HW_SetLED ( HW_LED_CAN4, HW_LED_GREEN);
	HW_SetLED ( HW_LED_CAN5, HW_LED_GREEN);
	HW_SetLED ( HW_LED_CAN6, HW_LED_GREEN);

  //main_greeting();
  for(;;)
  {
    HAL_IWDG_Refresh(&hiwdg);

	  CANTxMsg_t txMsg;

	  txMsg.bufftype = CAN_BUFFER_TX_MSG;
    txMsg.dlc      = CAN_LEN8_DLC;             // 8 bytes data length
    txMsg.msgtype  = CAN_MSGTYPE_EXTENDED | CAN_MSGTYPE_FDF | CAN_MSGTYPE_BRS;     // 표준 CAN FD 메시지 타입
    txMsg.id       = 0x12345678;                     // 테스트용 ID

    // 테스트 데이터 설정 (예: 8바이트)
    txMsg.data8[0] = 0xAA;
    txMsg.data8[1] = 0xBB;
    txMsg.data8[2] = 0xCC;
    txMsg.data8[3] = 0xDD;
    txMsg.data8[4] = 0x11;
    txMsg.data8[5] = 0x22;
    txMsg.data8[6] = 0x33;
    txMsg.data8[7] = 0x44;

    // 2. CAN_BUS1에 메시지 송신
    CAN_Write(CAN_BUS1, &txMsg);

		osDelay ( DLY_MS(1000));
  }
}


void task03_init(void *argument)
{
    (void)argument;
    DEBUG("TASK03 START\n");

    Task03Handle = osThreadNew(task03_thread, NULL, &Task03_attributes);

}