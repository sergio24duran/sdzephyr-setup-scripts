/*
 * Multithreaded LED Example - Main Entry Point
 *
 * This example demonstrates core RTOS (Real-Time Operating System) concepts
 * using Zephyr:
 *
 *   1. Threads        - Independent execution contexts with their own stacks
 *   2. Scheduling     - How the kernel decides which thread runs next
 *   3. Priorities     - Higher-priority threads preempt lower-priority ones
 *   4. Semaphores     - Synchronization primitives for coordinating threads
 *   5. K_THREAD_DEFINE - Static (compile-time) thread creation macro
 *   6. Kconfig        - Build-time configuration for all parameters
 *
 * Architecture overview:
 *
 *   +----------------+     gives semaphore     +----------------+
 *   |  Main Thread   | ----------------------> |  LED A Thread  |
 *   |  (boots first) | ---+                    |  (GPIO14, fast)|
 *   +----------------+    |                    +----------------+
 *                         | gives semaphore
 *                         +----------------->  +----------------+
 *                                              |  LED B Thread  |
 *                                              |  (GPIO18, slow)|
 *                                              +----------------+
 *
 * All three threads run concurrently. The main thread signals the LED
 * threads to start via a semaphore, then sleeps indefinitely. Each LED
 * thread blinks its own LED at its own configured interval.
 *
 * Build & flash (replace <board> with your target, e.g. esp_wrover_kit/esp32/procpu):
 *   west build -p -b <board> examples/multithreads_hello_leds --build-dir examples/multithreads_hello_leds/build
 *   west flash --build-dir examples/multithreads_hello_leds/build
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* ---------------------------------------------------------------------------
 * Semaphore Declaration
 * ---------------------------------------------------------------------------
 * A semaphore is a synchronization primitive. It has an internal counter:
 *   - k_sem_give() increments the counter (signals)
 *   - k_sem_take() decrements it (waits if counter is 0)
 *
 * Here we use it as a "start gate": both LED threads wait on this semaphore
 * before blinking. The main thread gives it twice (once per thread) to
 * release them. This ensures controlled startup ordering.
 *
 * K_SEM_DEFINE(name, initial_count, max_count)
 *   - initial_count = 0: threads must wait until main signals
 *   - max_count = 1: binary semaphore (only one give() can be pending)
 */
K_SEM_DEFINE(start_sem, 0, 1);

/* ---------------------------------------------------------------------------
 * External Thread Entry Functions
 * ---------------------------------------------------------------------------
 * These functions are defined in separate source files (led_a_thread.c and
 * led_b_thread.c). In Zephyr, all threads share the same address space,
 * so we just need forward declarations here.
 */
void led_a_entry(void *, void *, void *);
void led_b_entry(void *, void *, void *);

/* ---------------------------------------------------------------------------
 * Static Thread Definitions
 * ---------------------------------------------------------------------------
 * K_THREAD_DEFINE creates threads at compile time. This is more efficient
 * than creating threads at runtime with k_thread_create() because the
 * kernel can allocate stacks and thread structures statically.
 *
 * Parameters:
 *   1. Thread ID     - A symbolic name for referencing the thread
 *   2. Stack size    - Bytes of stack memory (from Kconfig)
 *   3. Entry function - The function the thread executes
 *   4-6. Arguments   - Three void* args passed to the entry function
 *   7. Priority      - Thread scheduling priority (from Kconfig)
 *   8. Options       - Thread options (0 = default)
 *   9. Start delay   - Milliseconds before the thread starts (0 = immediate)
 *
 * IMPORTANT: Lower priority NUMBER = Higher scheduling priority.
 *   Priority 5 runs BEFORE priority 7 when both are ready.
 */
K_THREAD_DEFINE(led_a_tid,
	CONFIG_LED_THREAD_STACK_SIZE,
	led_a_entry,
	NULL, NULL, NULL,
	CONFIG_LED_A_PRIORITY,
	0,
	0);

K_THREAD_DEFINE(led_b_tid,
	CONFIG_LED_THREAD_STACK_SIZE,
	led_b_entry,
	NULL, NULL, NULL,
	CONFIG_LED_B_PRIORITY,
	0,
	0);

/* ---------------------------------------------------------------------------
 * main() - Application Entry Point
 * ---------------------------------------------------------------------------
 * The main thread runs after kernel initialization is complete. Here it:
 *   1. Logs a startup message
 *   2. Signals the LED threads to start (via the semaphore)
 *   3. Sleeps forever, yielding CPU time to the LED threads
 *
 * Note: Even though main() loops with k_msleep(), the LED threads are
 * the ones doing the real work. The main thread could also perform
 * periodic tasks (monitoring, watchdog, etc.) in this loop.
 */
int main(void)
{
	LOG_INF("=== Multithreaded LED Example Started ===");
	LOG_INF("LED A: GPIO%d, interval %d ms, priority %d",
		CONFIG_LED_A_GPIO_PIN,
		CONFIG_LED_A_BLINK_INTERVAL_MS,
		CONFIG_LED_A_PRIORITY);
	LOG_INF("LED B: GPIO%d, interval %d ms, priority %d",
		CONFIG_LED_B_GPIO_PIN,
		CONFIG_LED_B_BLINK_INTERVAL_MS,
		CONFIG_LED_B_PRIORITY);

#if CONFIG_LED_SYNC_ENABLED
	/*
	 * Signal both threads to start. Each k_sem_give() releases one
	 * thread that is waiting on k_sem_take(&start_sem, K_FOREVER).
	 * We give twice because two threads are waiting.
	 */
	LOG_INF("Releasing LED threads via semaphore...");
	k_sem_give(&start_sem);
	k_sem_give(&start_sem);
#endif

	/* Main thread has nothing else to do - sleep forever */
	while (1) {
		k_msleep(10000);
	}

	return 0;
}
