/*
 * LED A Thread - Blinks LED on GPIO pin A
 *
 * This thread demonstrates:
 *   - Waiting on a semaphore before starting work
 *   - Independent GPIO control within a thread context
 *   - Using Kconfig values for all configurable parameters
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(led_a, LOG_LEVEL_INF);

/* The start semaphore is defined in main.c */
extern struct k_sem start_sem;

/*
 * led_a_entry() - Thread entry point for LED A
 *
 * The three void* parameters are passed from K_THREAD_DEFINE. We don't
 * use them in this example (they are NULL), but they can be used to pass
 * configuration data to threads when using k_thread_create() at runtime.
 */
void led_a_entry(void *p1, void *p2, void *p3)
{
	const struct device *gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));

	if (!device_is_ready(gpio_dev)) {
		LOG_ERR("GPIO device not ready");
		return;
	}

	int ret = gpio_pin_configure(gpio_dev, CONFIG_LED_A_GPIO_PIN,
				     GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure GPIO%d (error %d)",
			CONFIG_LED_A_GPIO_PIN, ret);
		return;
	}

#if CONFIG_LED_SYNC_ENABLED
	/*
	 * Wait for the main thread to give the semaphore.
	 * K_FOREVER means "block indefinitely until the semaphore is available".
	 * The thread is in WAITING state during this time and consumes no CPU.
	 */
	LOG_INF("LED A waiting for start signal...");
	k_sem_take(&start_sem, K_FOREVER);
#endif

	LOG_INF("LED A thread running (GPIO%d, every %d ms)",
		CONFIG_LED_A_GPIO_PIN, CONFIG_LED_A_BLINK_INTERVAL_MS);

	bool state = true;

	while (1) {
		gpio_pin_toggle(gpio_dev, CONFIG_LED_A_GPIO_PIN);
		state = !state;
		LOG_INF("LED A (GPIO%d): %s", CONFIG_LED_A_GPIO_PIN,
			state ? "ON" : "OFF");
		k_msleep(CONFIG_LED_A_BLINK_INTERVAL_MS);
	}
}
