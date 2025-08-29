
#include <stdint.h>
#include <stdlib.h>
#include "can.h"
#include "can_user.h"
#include "hardware.h"



// timings for CAN1 to CAN6
static const CANTiming_t  Timing_CANx[6] = {
_40M_500K_80____4M_80_ISO,	  // CAN1
_40M_500K_80____4M_80_ISO,	  // CAN2
_40M_500K_80____4M_80_ISO,	  // CAN3
_40M_250K_80____2M_80_ISO,	  // CAN4
_40M_500K_80____4M_80_ISO,	  // CAN5
_40M_500K_80____4M_80_ISO	  // CAN6
};





//! @brief      Wrapper used to pre-process some incoming data.
//!             PCAN-Router Pro FD will use a central receive cache with a
//!             single read interface for all CANs so we can not read from
//!             a specific CAN.
//! @param[in]  hBus  not used here (use CAN_BUSX)
//! @param[out] buff  buffer to read record into
//! @return     see CAN_ERR_...
CANResult_t  CAN_UserRead ( CANHandle_t  hBus, void  *buff)
{
	CANResult_t  ret;
	CANBuffer_t  *rx_buff;
	
	
	ret = CAN_ERR_RX_EMPTY;
	rx_buff = buff;
	
	if ( CAN_Read ( hBus, rx_buff) == CAN_ERR_OK)
	{
		// CAN bus is returned from data buffer due to central rx-cache
		hBus = rx_buff->hBus;
		
		// buffer read from CANx. Check type of buffer.
		switch ( rx_buff->bufftype)
		{
			case CAN_BUFFER_STATUS:
				// the buffer is a status notification from the CAN controller.
				if ( rx_buff->status.bus_status)
				{
					// CAN controller not involved into bus activities
					
					// uninitialize (this includes a TX-path flush)
					CAN_UnInitialize ( hBus);
					
					// initialize CAN controller
					CAN_Initialize ( hBus, &Timing_CANx[hBus]);
				}
				break;
				
				
			case CAN_BUFFER_RX_MSG:
				// the buffer is a receive message. Forward to application.
				ret = CAN_ERR_OK;
				break;
				
				
			case CAN_BUFFER_CRITICAL:
				// receive queue level was critical. Data might be lost.
				break;
		}
	}
	
	return ret;
}



//! @brief      invoke CAN bootloader
//! @param[in]  settings  select set of bitrates for CAN buses (see code below)
void  CAN_UserInvokeBootloader ( uint8_t  settings)
{
	if ( settings == 1)
	{
		// with this path the bootloader will use it's default timing
		HW_JumpToBootloader ( NULL);
	}

	else if ( settings == 2)
	{
		// with this path the bootloader will use the timings from the
		// list below.
		static const CANTiming_t  TimingForBootloader[6] = {
			_80M_125K_80____2M_80_ISO,    // CAN1
			_80M_125K_80____2M_80_ISO,    // CAN2
			_80M_125K_80____2M_80_ISO,    // CAN3
			_80M_125K_80____2M_80_ISO,    // CAN4
			_80M_125K_80____2M_80_ISO,    // CAN5
			_80M_125K_80____2M_80_ISO     // CAN6
		};

		HW_JumpToBootloader ( &TimingForBootloader[0]);
	}

	else if ( settings == 3)
	{
		// with this path the bootloader will use the timings from application.
		HW_JumpToBootloader ( &Timing_CANx[0]);
	}

}



//! @brief      init CAN1 to CAN6.
//!             extended IDs can not be filtered (always received all)
void  CAN_UserInit ( void)
{
	// init CAN1
	CAN_Initialize ( CAN_BUS1, &Timing_CANx[CAN_BUS1]);
	
	// receive all extended CAN-FD
	CAN_FilterAdd ( CAN_BUS1, CAN_MSGTYPE_EXTENDED | CAN_MSGTYPE_FDF, 0x00000000, 0x1FFFFFFF);
	
	
	
	// init CAN2
	CAN_Initialize ( CAN_BUS2, &Timing_CANx[CAN_BUS2]);
	
	// receive all extended CAN-FD
	CAN_FilterAdd ( CAN_BUS2, CAN_MSGTYPE_EXTENDED | CAN_MSGTYPE_FDF, 0x00000000, 0x1FFFFFFF);
	
	
	
	// init CAN3
	CAN_Initialize ( CAN_BUS3, &Timing_CANx[CAN_BUS3]);
	
	// receive all extended CAN-FD
	CAN_FilterAdd ( CAN_BUS3, CAN_MSGTYPE_EXTENDED | CAN_MSGTYPE_FDF, 0x00000000, 0x1FFFFFFF);
	
	
	
	// init CAN4
	CAN_Initialize ( CAN_BUS4, &Timing_CANx[CAN_BUS4]);
	
	// receive all extended CAN-FD
	CAN_FilterAdd ( CAN_BUS4, CAN_MSGTYPE_STANDARD | CAN_MSGTYPE_FDF, 0x000, 0x7FF);
	
	
	
	// init CAN5
	CAN_Initialize ( CAN_BUS5, &Timing_CANx[CAN_BUS5]);
	
	// receive all extended CAN-FD
	CAN_FilterAdd ( CAN_BUS5, CAN_MSGTYPE_EXTENDED | CAN_MSGTYPE_FDF, 0x00000000, 0x1FFFFFFF);
	
	
	
	// init CAN6
	CAN_Initialize ( CAN_BUS6, &Timing_CANx[CAN_BUS6]);
	
	// receive all extended CAN-FD
	CAN_FilterAdd ( CAN_BUS6, CAN_MSGTYPE_EXTENDED | CAN_MSGTYPE_FDF, 0x00000000, 0x1FFFFFFF);
}

