
#include <stdint.h>
#include <stdlib.h>
#include "ARMCM7_DP.h"
#include "can.h"
#include "eeprom.h"
#include "stm32f765xx.h"
#include "hardware.h"

// v1  first version
// v2  support added for eeprom at ethernet PHY board. Eeprom is selected
//     by address bit A18 ( M24M02 from st micro).
//     0x00000 - 0x3FFFF  ( 2MBit main eeprom at PCAN-Router Pro FD)
//     0x40000 - 0x7FFFF  ( 2MBit second eeprom at ethernet PHY board)
// v3  small mods due to changed CPU settings from bootloader 2.0.9 .
//     Typical changes are memory barriers to ensure memory access order.
//     Barriers are set preventive, maybe more are needed ...

#define	EEPROM_DEV_ADDR		0xA0

// do not use 256 !!
#define	EEPROM_CACHE_SIZE		128

// total size is 2 MBit (256 KByte) per i2c device
#define	EEPROM_MEM_SIZE		( 256 * 1024 * 2)

#define	PAGE_MASK				( EEPROM_CACHE_SIZE - 1)

#define SAMEPAGE( addr1, addr2)	((addr1 | PAGE_MASK) == (addr2 | PAGE_MASK))



void  EEPROM_Init ( void);


typedef struct {
	uint32_t		startaddr;
	uint32_t		count;
	uint8_t		dirty;
	uint8_t		data[EEPROM_CACHE_SIZE];
} EECache_t;


// local eeprom cache
static EECache_t Cache;

// remember last byte for eeprom interaction
static uint8_t  LastCntlByte;





//! @brief      check for ACK from eeprom
static EEPROMResult_t ackpoll ( void)
{
	EEPROMResult_t  stat;
	uint32_t  isr;


	stat = EEPROM_ERR_OK;
	

	while ( I2C1->ISR & ( 3 << 4))
	 { I2C1->ICR = 3 << 4;}


	// start
	I2C1->CR2 = 1 << 25 | 0 << 16 | LastCntlByte;
	__DSB();
	I2C1->CR2 |= 1 << 13;


	do {
		isr = I2C1->ISR;

		if ( isr & ( 1 << 4))
		{
			// NACK
			stat = EEPROM_ERR_ILLDEVICE;
		}
		
	// until stop
	} while ( !( isr & ( 1 << 5)));

	return stat;
}



//! @brief      write cachepage back to eeprom
static EEPROMResult_t writecachepage ( void)
{
	EEPROMResult_t  ret;
	uint32_t  i;


	ret = EEPROM_ERR_OK;


	if ( !Cache.dirty || Cache.count == 0)
	{
		// nothing to write
		Cache.dirty = 0;

		return ret;
	}

	
	// wait until eeprom ready
	while ( ackpoll() != EEPROM_ERR_OK)
	 {}



	while ( I2C1->ISR & ( 1 << 5 | 1 << 1))
	 { I2C1->ICR = 1 << 5 | 1 << 1;}
	


	// start
	LastCntlByte = EEPROM_DEV_ADDR | (( Cache.startaddr >> 15) & 0x0E);
	I2C1->CR2 = 0 << 25 | ( 2 + Cache.count) << 16 | 0 << 10 | LastCntlByte;
	__DSB();
	I2C1->CR2 |= 1 << 13;


	while ( !( I2C1->ISR & ( 1 << 1)))
	 {}

	I2C1->TXDR = ( Cache.startaddr >> 8) & 0xFF;


	while ( !( I2C1->ISR & ( 1 << 1)))
	 {}

	I2C1->TXDR = ( Cache.startaddr >> 0) & 0xFF;


	// send data bytes
	for ( i = 0; i < Cache.count; i++)
	{
		while ( !( I2C1->ISR & ( 1 << 1)))
		 {}

		I2C1->TXDR = Cache.data[i];
	}


	// wait for last byte
	while ( !( I2C1->ISR & ( 1 << 6)))
	 {}

	// initiate STOP
	I2C1->CR2 = 0;
	__DSB();
	I2C1->CR2 = 1 << 14;

	// wait for STOP
	while ( !( I2C1->ISR & ( 1 << 5)))
	 {}


	if ( ret == EEPROM_ERR_OK)
	 { Cache.dirty = 0;}		// cache written


	return ret;
}



//! @brief      read page from eeprom into cache
static  EEPROMResult_t readcachepage ( uint32_t Addr, uint32_t count)
{
	EEPROMResult_t ret;


	ret = EEPROM_ERR_OK;
	

	if ( Cache.dirty)			// write cache if dirty
	{
		ret = writecachepage();
		
		if ( ret != EEPROM_ERR_OK)
		{
			return ret;
		}
	}
	

	Cache.startaddr = Addr;
	Cache.count = count;
	
	// wait until eeprom ready
	while ( ackpoll() != EEPROM_ERR_OK)
	 {}


	while ( I2C1->ISR & ( 1 << 6 | 1 << 5 | 1 << 2 | 1 << 1))
	 { I2C1->ICR = 1 << 6 | 1 << 5 | 1 << 2 | 1 << 1;}
	


	// start
	LastCntlByte = EEPROM_DEV_ADDR | (( Cache.startaddr >> 15) & 0x0E);
	I2C1->CR2 = 2 << 16 | 0 << 10 | LastCntlByte;
	__DSB();
	I2C1->CR2 |= 1 << 13;


	while ( !( I2C1->ISR & ( 1 << 1)))
	 {}

	I2C1->TXDR = ( Cache.startaddr >> 8) & 0xFF;


	while ( !( I2C1->ISR & ( 1 << 1)))
	 {}

	I2C1->TXDR = ( Cache.startaddr >> 0) & 0xFF;


	while ( !( I2C1->ISR & ( 1 << 6)))
	 {}


	
	// repeated start
	LastCntlByte = EEPROM_DEV_ADDR | (( Cache.startaddr >> 15) & 0x0E);
	I2C1->CR2 = 0 << 25 | count << 16 | 1 << 10 | LastCntlByte;
	__DSB();
	I2C1->CR2 |= 1 << 13;


	// read data
	if ( Cache.count > 1) 
	{
		uint32_t  i;

		
		for ( i = 0; i < Cache.count; i++) 
		{
			while ( !( I2C1->ISR & ( 1 << 2)))
			 {}

			Cache.data[i] = I2C1->RXDR;
		}
	}


	// initiate STOP
	I2C1->CR2 = 0;
	__DSB();
	I2C1->CR2 = 1 << 14;


	// wait for STOP
	while ( !( I2C1->ISR & ( 1 << 5)))
	 {}


	if ( ret == EEPROM_ERR_OK)
	 { Cache.dirty = 0;} // data read to cache


	return ret;
}



//! @brief      read data from eeprom into a user buffer
//! @param[in]  addr    address within eeprom memory
//! @param[out] buffer  user buffer to return eeprom data
//! @param[in]  length  number of bytes read from eeprom
EEPROMResult_t  EEPROM_Read ( uint32_t addr, void * buffer, uint32_t length)
{
	EEPROMResult_t ret;
	uint8_t  *pBuff;


	ret = EEPROM_ERR_OK;

	pBuff = buffer;

	if ( addr >= EEPROM_MEM_SIZE)
	{
		ret = EEPROM_ERR_ILLPARAMVAL;
		goto exit;
	}


	if ( length == 0)
	{
		ret = EEPROM_ERR_ILLPARAMVAL;
		goto exit;
	}


	if (( addr + length) > EEPROM_MEM_SIZE)
	{
		ret = EEPROM_ERR_ILLPARAMVAL;
		goto exit;
	}


	if ( buffer == NULL)
	{
		ret = EEPROM_ERR_ILLPARAMVAL;
		goto exit;
	}

	
	if ( Cache.dirty)
	{
		ret = writecachepage();
		
		if ( ret != EEPROM_ERR_OK)
		{
			goto exit;
		}
	}
	

	while ( length)
	{
		if (( addr < Cache.startaddr) || ( addr - Cache.startaddr >= Cache.count))
		{
			ret = readcachepage ( addr & ~( EEPROM_CACHE_SIZE - 1), EEPROM_CACHE_SIZE);
				
            if ( ret != EEPROM_ERR_OK)
            {
            	goto exit;
			}
		}

		*pBuff++ = Cache.data[addr - Cache.startaddr];
		addr++;
		length--;
	}

	exit:
	return ret;
}



//! @brief      write data from user buffer into eeprom
//! @param[in]  addr    address within eeprom memory
//! @param[in]  buffer  user buffer containing write data
//! @param[in]  length  number of bytes to write to eeprom
EEPROMResult_t  EEPROM_Write ( uint32_t addr, void * buffer, uint32_t length)
{
	EEPROMResult_t ret;
	uint8_t  *pBuff;


	ret = EEPROM_ERR_OK;

	pBuff = buffer;

	
	if ( addr >= EEPROM_MEM_SIZE)
	{
		ret = EEPROM_ERR_ILLPARAMVAL;
		goto exit;
	}


	if ( length == 0)
	{
		ret = EEPROM_ERR_ILLPARAMVAL;
		goto exit;
	}


	if (( addr + length) > EEPROM_MEM_SIZE)
	{
		ret = EEPROM_ERR_ILLPARAMVAL;
		goto exit;
	}


	if ( buffer == NULL)
	{
		ret = EEPROM_ERR_ILLPARAMVAL;
		goto exit;
	}

	
	while ( length) 
	{
		if ( !SAMEPAGE ( addr, Cache.startaddr) || ( addr < Cache.startaddr) || ( addr > Cache.startaddr + Cache.count))
		{
			ret = writecachepage();
			
			if ( ret != EEPROM_ERR_OK)
			{
				goto exit;
			}
		}


		if ( !Cache.dirty)
		{
			// remember new write page
			Cache.startaddr = addr;
			Cache.dirty = 1;
			Cache.count = 0;
		}

		// extend cache if addr at this boundary
		if ( addr == Cache.startaddr + Cache.count)
		 { Cache.data[Cache.count++] = *pBuff++;}


		length--;
		addr++;
	}

	exit:
	return ret;
}



//! @brief      flush data from cache into eeprom
EEPROMResult_t  EEPROM_FlushCache ( void)
{
	EEPROMResult_t ret;


	ret = writecachepage();
	
	if ( ret != EEPROM_ERR_OK)
	{
		goto exit;
	}

	// wait until all data written
	while ( ackpoll() != EEPROM_ERR_OK)
	 {}

	Cache.count = 0;

	exit:
	return ret;
}



//! @brief      initial at startup. Called from HW_Init()
void  EEPROM_Init ( void)
{

	Cache.dirty = 0;
	Cache.count = 0;
	Cache.startaddr = 0xFFFFFFFF;

	LastCntlByte = EEPROM_DEV_ADDR;

	ackpoll();
}



