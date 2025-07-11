/**
 ******************************************************************************
 * @file    LwIP/LwIP_IAP/Src/flash_if.c
 * @author  MCD Application Team
 * @brief   This file provides high level routines to manage internal Flash
 *          programming (erase and write).
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2017 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "flash_if.h"
#include "stm32f7xx_hal_flash.h"
#include <stdint.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Unlocks Flash for write access
 * @param  None
 * @retval None
 */
uint32_t GetSectorNumber(uint32_t Address)
{
    if (Address < 0x08008000) return FLASH_SECTOR_0;  // 32KB
    if (Address < 0x08010000) return FLASH_SECTOR_1;  // 32KB
    if (Address < 0x08018000) return FLASH_SECTOR_2;  // 32KB
    if (Address < 0x08020000) return FLASH_SECTOR_3;  // 32KB
    if (Address < 0x08040000) return FLASH_SECTOR_4;  // 128KB
    if (Address < 0x08080000) return FLASH_SECTOR_5;  // 256KB
    if (Address < 0x080C0000) return FLASH_SECTOR_6;  // 256KB
    if (Address < 0x08100000) return FLASH_SECTOR_7;  // 256KB
	if (Address < 0x08140000) return FLASH_SECTOR_8;  // 256KB
	if (Address < 0x08180000) return FLASH_SECTOR_9;  // 256KB
	if (Address < 0x081C0000) return FLASH_SECTOR_10;  // 256KB
	if (Address < 0x08200000) return FLASH_SECTOR_11;  // 256KB
    return FLASH_SECTOR_11; //
}


void FLASH_If_Init(void) {
	HAL_FLASH_Unlock();
}

/**
 * @brief  This function does an erase of all user flash area
 * @param  StartSector: start of user flash area
 * @retval 0: user flash area successfully erased
 *         1: error occured
 */
int8_t FLASH_If_Erase(uint32_t StartSector) {
	uint32_t FlashAddress;

	FlashAddress = StartSector;

	/* Device voltage range supposed to be [2.7V to 3.6V], the operation will
	 be done by word */

	if (FlashAddress <= (uint32_t) USER_FLASH_LAST_PAGE_ADDRESS) {
		FLASH_EraseInitTypeDef FLASH_EraseInitStruct;
		uint32_t sectornb = 0;

		FLASH_EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
		FLASH_EraseInitStruct.Sector = GetSectorNumber(FlashAddress);
		FLASH_EraseInitStruct.NbSectors = GetSectorNumber(USER_FLASH_LAST_PAGE_ADDRESS) - GetSectorNumber(FlashAddress) + 1;
		FLASH_EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

		if (HAL_FLASHEx_Erase(&FLASH_EraseInitStruct, &sectornb) != HAL_OK)
			return (1);
	} else {
		return (1);
	}

	return (0);
}
/**
 * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
 * @note   After writing data buffer, the flash content is checked.
 * @param  FlashAddress: start address for writing data buffer
 * @param  Data: pointer on data buffer
 * @param  DataLength: length of data buffer (unit is 32-bit word)
 * @retval 0: Data successfully written to Flash memory
 *         1: Error occurred while writing data in Flash memory
 *         2: Written Data in flash memory is different from expected one
 */
uint32_t FLASH_If_Write(__IO uint32_t *FlashAddress, uint32_t *Data,
		uint16_t DataLength) {
	uint32_t i = 0;

	for (i = 0;
			(i < DataLength) && (*FlashAddress <= (USER_FLASH_END_ADDRESS - 4));
			i++) {
		/* Device voltage range supposed to be [2.7V to 3.6V], the operation will
		 be done by word */
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, *FlashAddress,
				*(uint32_t*) (Data + i)) == HAL_OK) {
			/* Check the written value */
			if (*(uint32_t*) *FlashAddress != *(uint32_t*) (Data + i)) {
				/* Flash content doesn't match SRAM content */
				return (2);
			}
			/* Increment FLASH destination address */
			*FlashAddress += 4;
		} else {
			/* Error occurred while writing data in Flash memory */
			return (1);
		}
	}

	return (0);
}

