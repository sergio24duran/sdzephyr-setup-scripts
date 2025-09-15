/*
 * Multithreads Blinky on ESP32 DevKitC using GPIO14 and GPIO18
 * Zephyr RTOS
 */

#include <zephyr/kernel.h>

/* Forward declarations of the thread entry functions */
void led14_thread(void);
void led18_thread(void);

/* 
 * Define the threads using K_THREAD_DEFINE:
 * - thread ID (symbolic name for the thread)
 * - stack size in bytes (1024)
 * - entry function (function executed by the thread)
 * - 3 optional arguments (NULL, since we don't pass any)
 * - priority (7 in this case, smaller number = higher priority)
 * - options (0 = default)
 * - start delay (0 = start immediately)
 */
K_THREAD_DEFINE(led14_tid, 1024, led14_thread, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(led18_tid, 1024, led18_thread, NULL, NULL, NULL, 7, 0, 0);

int main(void)
{
    /* 
    * The main thread does not need to explicitly "start" led2_thread 
    * or led18_thread. The K_THREAD_DEFINE macro already created 
    * and scheduled them at boot.
    *
    * Here we simply keep main alive (it could also perform other tasks).
    */
    while (1) {
        k_msleep(1000);
    }
    return 0;
}
