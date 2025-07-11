#include "backup.h"
#include "main.h"
#include "flash_if.h"
#include "tftpserver.h"
#include "firmware_tag.h"

#include "stm32f7xx_hal_flash_ex.h"
#include "stm32f7xx_hal_crc.h"
#include "stm32f7xx_hal_crc_ex.h"

#define TAG_ADDRESS  ((FirmwareTag*) USER_FLASH_FIRST_PAGE_ADDRESS)
int8_t FLASH_Backup_Erase(uint32_t StartSector) {
	uint32_t FlashAddress;

	FlashAddress = StartSector;

	/* Device voltage range supposed to be [2.7V to 3.6V], the operation will
	 be done by word */

	if (FlashAddress <= (uint32_t) BACKUP_FLASH_LAST_PAGE_ADDRESS) {
		FLASH_EraseInitTypeDef FLASH_EraseInitStruct;
		uint32_t sectornb = 0;

		FLASH_EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
		FLASH_EraseInitStruct.Sector = GetSectorNumber(FlashAddress);
		FLASH_EraseInitStruct.NbSectors = GetSectorNumber(BACKUP_FLASH_LAST_PAGE_ADDRESS) - GetSectorNumber(FlashAddress) + 1;
		FLASH_EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

		if (HAL_FLASHEx_Erase(&FLASH_EraseInitStruct, &sectornb) != HAL_OK)
			return (1);
	} else {
		return (1);
	}

	return (0);
}


uint32_t CAL_CRC(CRC_HandleTypeDef hcrc)
{
	HAL_CRC_DeInit(&hcrc);
	HAL_CRC_Init(&hcrc);
	__HAL_CRC_DR_RESET(&hcrc);
	uint32_t* bin_data = (uint32_t*)(USER_FLASH_FIRST_PAGE_ADDRESS + 0x400);  
	uint32_t word_count = TAG_ADDRESS->size  / 4; //TAG_ADDRESS->size 
	uint32_t i;
	for (i = 0; i < word_count; i++)
	{
	   	hcrc.Instance->DR = bin_data[i];
	}

	uint32_t remaining =TAG_ADDRESS->size  % 4; //TAG_ADDRESS->size 
	if (remaining)
	{
	   uint8_t* byte_ptr = (uint8_t*)&bin_data[word_count];
	   for (i = 0; i < remaining; i++)
	   {
	      *(__IO uint8_t*)(&hcrc.Instance->DR) = byte_ptr[i];
	   }
	}
	return hcrc.Instance->DR;
}

// uint32_t CAL_CRC(CRC_HandleTypeDef hcrc)
// {
//     HAL_CRC_DeInit(&hcrc);
//     HAL_CRC_Init(&hcrc);

//     uint32_t* bin_data = (uint32_t*)(USER_FLASH_FIRST_PAGE_ADDRESS + 0x400);
    
//     char msg[64];
//     sprintf(msg, "bin_data[0] = 0x%08lX\r\n", bin_data[0]);
//     udp_client_send(msg);

//     return HAL_CRC_Calculate(&hcrc, bin_data, 1);  // 1 word = 4 bytes
// }


