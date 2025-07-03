
#ifndef _EEPROM_H_
#define _EEPROM_H_

//
//	eeprom.h
//
//	-------------------------------------------------------------------
//! @addtogroup EEPROM
//! <h3> API Functions for eeprom access </h3>
//!
//! The functions can be used to access eeprom memory to
//! store non-volatile data. The functions are not interrupt protected
//! and will target to the main() level. If the user want to write from
//! interrupts he has to build own wrapper functions for interrupt safety.
//! All functions are made in a polling style. So the speed of execution
//! is limited to the physical speed of the communication media.
//! Typically this will be an internal interface @ 375 KHz.
//! Write-data is cached internally until a cache-miss is detected. Cache
//! will be handled internally too. Make a call to EEPROM_FlushCache() to
//! save critical data before it is lost by power removal.
//!
//! <h3> Targets: </h3>
//! - PCAN-Router FD ( 4032 bytes)
//!
//!
//	-------------------------------------------------------------------
//
//	Copyright (C) PEAK-System Technik GmbH, Darmstadt
//	more Info at http://www.peak-system.com 
//
//! @{


////////////////////////////////////////////////////////////
//! @name errors
//! A function returns one of these errors:
//! @{
#define	EEPROM_ERR_OK				0		//!< OK, no error
#define	EEPROM_ERR_ILLDEVICE		1		//!< device not responding
#define	EEPROM_ERR_ILLPARAMVAL	2		//!< invalid parameter value
#define	EEPROM_ERR_UNKNOWN		3		//!< unknown error
#define EEPROM_ERR_TIMEOUT      4       //add new something 
#include <stdint.h>
/*! @}*/


////////////////////////////////////////////////////////////
//! @name types
//! Basetypes for eeprom functions.
//! @{
#define	EEPROMResult_t		uint32_t			//!< status type for API functions
/*! @}*/


#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////
//! @name functions
//! This section will describe the API functions. The functions will target
//! to the main() level.
//! @{


//! @brief
//! Read data from the eeprom.
//!
//! @param		addr			eeprom address to read from
//! @param		buffer		user buffer to write data to
//! @param		length		number of bytes to read
//!
//! @return		one error of EEPROM_ERR_...
EEPROMResult_t		EEPROM_Read (	uint32_t  addr, void  *buffer, uint32_t  length);



//! @brief
//! Write data to the eeprom.
//!
//! @param		addr			eeprom address to write data to
//! @param		buffer		user buffer to read data from
//! @param		length		number of bytes to write
//!
//! @return		one error of EEPROM_ERR_...
EEPROMResult_t		EEPROM_Write (	uint32_t  addr, void  *buffer, uint32_t  length);



//! @brief
//! Flush the cache to the eeprom.
//!
//! @return		one error of EEPROM_ERR_...
EEPROMResult_t		EEPROM_FlushCache ( void);

/*! @}*/

#ifdef __cplusplus
}
#endif

/*! @}*/

#endif
