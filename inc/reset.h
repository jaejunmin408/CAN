#include <stdint.h>

void write_boot_flag_soft_reset(void);
void clear_boot_flag(void);
uint8_t is_soft_reset(void);
void MX_RTC_Init(void);
