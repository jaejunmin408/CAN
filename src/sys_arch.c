
#include <lwip/arch.h>
#include "arch/sys_arch.h"
#include <lwip/sys.h>
#include <string.h>
#include "main.h"




static void  sys_arch_dbg ( uint32_t  err)
{
   //while(1);
}



void HAL_Delay(uint32_t Delay)
{
	osDelay ( DLY_MS(Delay));
}



uint32_t HAL_GetTick(void)
{
	return sys_now();
}







u32_t  sys_now ( void)
{
	return osKernelGetTickCount() / OS_TICKS_PER_MILLI;
}



void  sys_init ( void)
{
}




/**
 * @ingroup dhcp4
 * Start DHCP negotiation for a network interface.
 *
 * If no DHCP client instance was attached to this interface,
 * a new client is created first. If a DHCP client instance
 * was already present, it restarts negotiation.
 *
 * @param netif The lwIP network interface
 * @return lwIP error code
 * - ERR_OK - No error
 * - ERR_MEM - Out of memory
 */
sys_thread_t  sys_thread_new ( const char *name, lwip_thread_fn function, void *arg, int stacksize, int prio)
{
	osThreadId_t    thread_id;
	osThreadAttr_t  thread_attr;
	
	
	memset ( &thread_attr, 0, sizeof ( thread_attr));
	
	thread_attr.stack_size = stacksize;
	thread_attr.name = name;
	thread_attr.priority = prio;
	
	thread_id = osThreadNew ( function, arg, &thread_attr);
	
	return thread_id;
}



err_t  sys_mutex_new ( sys_mutex_t *mutex)
{
   static const osMutexAttr_t attr = {
      NULL,
      osMutexPrioInherit | osMutexRecursive,
      NULL,
      0
   };


   *mutex = osMutexNew ( &attr);

   if ( *mutex)
      return ERR_OK;

   sys_arch_dbg ( 1);
   
   return ERR_MEM;
}



void  sys_mutex_lock ( sys_mutex_t *mutex)
{
   osStatus_t  stat;


	stat = osMutexAcquire ( *mutex, osWaitForever);

   if ( stat != osOK) {
      sys_arch_dbg ( 2);
   }
}



void  sys_mutex_unlock ( sys_mutex_t *mutex)
{
	osStatus_t  stat;


	stat = osMutexRelease ( *mutex);

   if ( stat != osOK) {
      sys_arch_dbg ( 3);
   }
}



err_t  sys_mbox_new ( sys_mbox_t *mbox, int size)
{
	
	*mbox = osMessageQueueNew ( size, sizeof ( void*), NULL);
	
	if ( *mbox)
	 return ERR_OK;
	
   sys_arch_dbg ( 4);
	return ERR_MEM;
}



void  sys_mbox_post ( sys_mbox_t *q, void *msg)
{
	uint32_t  tmp;
   osStatus_t  stat;
	
	
	tmp = (uint32_t) msg;
	stat = osMessageQueuePut ( *q, &tmp, 0, osWaitForever);

   if ( stat != osOK) {
      sys_arch_dbg ( 5);
   }
}



err_t  sys_mbox_trypost ( sys_mbox_t *q, void *msg)
{
	uint32_t  tmp;
	
	
	tmp = (uint32_t) msg;
	
	if ( osMessageQueuePut ( *q, &tmp, 0, 0) == osOK)
	 return ERR_OK;
	
   sys_arch_dbg ( 6);
	return ERR_MEM;
}



err_t  sys_mbox_trypost_fromisr ( sys_mbox_t *q, void *msg)
{
	return sys_mbox_trypost ( q, msg);
}



u32_t  sys_arch_mbox_fetch ( sys_mbox_t *q, void **msg, u32_t timeout)
{
	osStatus_t  stat;
	uint32_t  tmp;
	
	
	if ( timeout)
	 stat = osMessageQueueGet (*q, &tmp, NULL, DLY_MS(timeout));
	else
	 stat = osMessageQueueGet (*q, &tmp, NULL, osWaitForever);
	
	
	if ( stat == osErrorTimeout)
	 return SYS_ARCH_TIMEOUT;
	
	*msg = (void*) tmp;
	
	return 0;
}




err_t  sys_sem_new ( sys_sem_t *sem, u8_t count)
{
   osSemaphoreId_t id;

   id = osSemaphoreNew ( 1U, count, NULL);

   if ( id == NULL) {
      sys_arch_dbg ( 7);
      return ERR_MEM;
   }

   *sem = id;

   return ERR_OK;
}



void  sys_mbox_free ( sys_mbox_t *mbox)
{
   osStatus_t  stat;


   stat = osMessageQueueDelete ( *mbox);

   if ( stat != osOK) {
      sys_arch_dbg ( 8);
   }
}



void  sys_sem_signal ( sys_sem_t *sem)
{
   osStatus_t  stat;


   stat = osSemaphoreRelease ( *sem);

   if ( stat != osOK) {
      sys_arch_dbg ( 9);
   }
}



void  sys_sem_free ( sys_sem_t *sem)
{
   osStatus_t  stat;


   stat = osSemaphoreDelete ( *sem);

   if ( stat != osOK) {
      sys_arch_dbg ( 10);
   }
}




u32_t  sys_arch_mbox_tryfetch ( sys_mbox_t *q, void **msg)
{
   osStatus_t status;
   uint32_t  tmp;


   status = osMessageQueueGet ( *q, &tmp, NULL, 0U);
   
   if (status != osOK) {
      return SYS_MBOX_EMPTY;
   }

   *msg = (void*) tmp;

   return 0U;
}




u32_t  sys_arch_sem_wait ( sys_sem_t *sem, u32_t timeout)
{
   osStatus_t status;


   if ( timeout)
      status = osSemaphoreAcquire ( *sem, DLY_MS(timeout));
   else
      status = osSemaphoreAcquire ( *sem, osWaitForever);
   

   if (status != osOK) {
      return SYS_ARCH_TIMEOUT;
   }
   
   return 0U;
}



void  sys_mutex_free ( sys_mutex_t *mutex)
{
   osStatus_t status;


   status = osMutexDelete ( *mutex);

   if ( status != osOK) {
      sys_arch_dbg ( 11);
   }
}








#if 0




u32_t
sys_jiffies(void)
{
  return osKernelGetTickCount();
}


test_sys_arch_waiting_fn the_waiting_fn;

void
test_sys_arch_wait_callback(test_sys_arch_waiting_fn waiting_fn)
{
  the_waiting_fn = waiting_fn;
}



#if LWIP_NETCONN_SEM_PER_THREAD
#error LWIP_NETCONN_SEM_PER_THREAD==1 not supported
#endif /* LWIP_NETCONN_SEM_PER_THREAD */
#endif


