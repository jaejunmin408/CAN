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
    /** Enable access to RTC and Backup domain */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR1 |= PWR_CR1_DBP; 
    // __HAL_RCC_PWR_CLK_ENABLE();
    // HAL_PWR_EnableBkUpAccess();  // 백업 도메인 접근 허용

    RCC->BDCR |= RCC_BDCR_RTCEN;           // RTC 클럭 Enable

    //__HAL_RCC_LSE_CONFIG(RCC_LSE_ON); // 또는 RCC_LSI_ON

    // while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET)
    // {
    //     // wait until LSE is ready
    // }

    // /** Select RTC clock source */
    // __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);  // LSE를 RTC 클럭 소스로 사용

    // /** Enable RTC Clock */
    // __HAL_RCC_RTC_ENABLE();

    // /** Initialize RTC with default values */
    // hrtc.Instance = RTC;
    // hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    // hrtc.Init.AsynchPrediv = 127;
    // hrtc.Init.SynchPrediv = 255;
    // hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    // hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    // hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    // if (HAL_RTC_Init(&hrtc) != HAL_OK)
    // {
    //     Error_Handler();  // 에러 처리 루틴
    // }
}