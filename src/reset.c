#include "ARMCM7_DP.h"
#include "reset.h"
#include "main.h"
#include <stm32f765xx.h>


#define BOOT_FLAG_BKUP_REG RTC_BKP_DR0
#define BOOT_FLAG_SOFT_RESET 0xA5A5
#define BOOT_FLAG_NORMAL     0x0000

void write_boot_flag_soft_reset(void)
{
    RTC->BKP1R = BOOT_FLAG_SOFT_RESET;  // BPK1R = DR1
}

void clear_boot_flag(void)
{
    RTC->BKP1R = 0;
}

uint8_t is_soft_reset(void)
{
    uint32_t val = RTC->BKP1R;
    return (val == BOOT_FLAG_SOFT_RESET);
}


void MX_RTC_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR1 |= PWR_CR1_DBP; 
    RCC->BDCR |= RCC_BDCR_RTCEN;           // RTC 클럭 Enabl
}