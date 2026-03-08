/*
 * LED B Thread - Blinks LED on GPIO pin B
 *
 * This thread is structurally identical to LED A but uses different
 * Kconfig settings (pin, interval, priority). This demonstrates how
 * the same logic can run in parallel with different configurations -
 * a common pattern in RTOS applications (e.g., multiple sensor readers,
 * multiple communication channels).
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(led_b, LOG_LEVEL_INF);

/* The start semaphore is defined in main.c */
extern struct k_sem start_sem;

void led_b_entry(void *p1, void *p2, void *p3)
{
	const struct device *gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));

	if (!device_is_ready(gpio_dev)) {
		LOG_ERR("GPIO device not ready");
		return;
	}

	int ret = gpio_pin_configure(gpio_dev, CONFIG_LED_B_GPIO_PIN,
				     GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure GPIO%d (error %d)",
			CONFIG_LED_B_GPIO_PIN, ret);
		return;
	}

#if CONFIG_LED_SYNC_ENABLED
	LOG_INF("LED B waiting for start signal...");
	k_sem_take(&start_sem, K_FOREVER);
#endif

	LOG_INF("LED B thread running (GPIO%d, every %d ms)",
		CONFIG_LED_B_GPIO_PIN, CONFIG_LED_B_BLINK_INTERVAL_MS);

	bool state = true;

	while (1) {
		gpio_pin_toggle(gpio_dev, CONFIG_LED_B_GPIO_PIN);
		state = !state;
		LOG_INF("LED B (GPIO%d): %s", CONFIG_LED_B_GPIO_PIN,
			state ? "ON" : "OFF");
		k_msleep(CONFIG_LED_B_BLINK_INTERVAL_MS);
	}
}
