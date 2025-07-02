/*
 * Copyright (c) 2017 Simon Goldschmidt
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Simon Goldschmidt
 *
 */
#ifndef LWIP_HDR_TEST_SYS_ARCH_H
#define LWIP_HDR_TEST_SYS_ARCH_H


#include <cmsis_os2.h>
//#include "core_cm7.h"
//#include "stm32f765xx.h"
#include "stm32f7xx_hal.h"


#define SYS_ARCH_DECL_PROTECT(lev)		uint32_t  lev
#define SYS_ARCH_PROTECT(lev)				lev = __get_PRIMASK(); __disable_irq(); __DSB(); __ISB()
#define SYS_ARCH_UNPROTECT(lev)			if ( !lev) __enable_irq()


typedef osThreadId_t                sys_thread_t;
typedef osMessageQueueId_t          sys_mbox_t;
typedef osMutexId_t                 sys_mutex_t;
typedef osSemaphoreId_t             sys_sem_t;


#define sys_mbox_valid(mbox)                 (((mbox) != NULL) && (*(mbox) != NULL))
#define sys_mbox_set_invalid(mbox)           if ((mbox) != NULL) { *(mbox) = NULL; }

#define sys_sem_valid(sema)                  ((sema) != NULL)
#define sys_sem_set_invalid(sema)            if ((sema) != NULL) { *(sema) = NULL; }

#define sys_mutex_valid(mutex)               (((mutex) != NULL) && (*(mutex) != NULL))
#define sys_mutex_set_invalid(mutex)         if ((mutex) != NULL) { *(mutex) = NULL; }



#if 0



struct lwip_mbox {
  void* sem;
  void** q_mem;
  unsigned int head, tail;
  int size;
  int used;
};

#define SYS_MBOX_NULL NULL

#define sys_mbox_valid_val(mbox) (((mbox).sem != NULL)  && ((mbox).sem != (void*)-1))


/* to implement doing something while blocking on an mbox or semaphore:
 * pass a function to test_sys_arch_wait_callback() that returns
 * '0' if waiting again and
 * '1' if now there should be something to do (used for asserting)
 */
typedef int (*test_sys_arch_waiting_fn)(sys_sem_t* wait_sem, sys_mbox_t* wait_mbox);
void test_sys_arch_wait_callback(test_sys_arch_waiting_fn waiting_fn);

/* current time */
extern u32_t lwip_sys_now;
#endif

#endif /* LWIP_HDR_TEST_SYS_ARCH_H */

