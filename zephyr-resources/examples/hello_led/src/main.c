/*
 * Hello LED - Blinky Example for Zephyr RTOS
 *
 * This is a minimal "Hello World" for embedded systems: blinking an LED.
 * It demonstrates several fundamental Zephyr concepts:
 *
 *   1. GPIO driver API    - How to configure and control hardware pins
 *   2. Kernel sleep       - How to yield the CPU using k_msleep()
 *   3. Kconfig options    - How build-time configuration controls behavior
 *   4. Logging subsystem  - How to use Zephyr's structured logging (LOG_INF)
 *   5. Main thread        - The entry point that Zephyr calls after boot
 *
 * Hardware: Any board with a GPIO-connected LED (ESP32, nRF52, STM32, etc.)
 * Default pin: GPIO14 (configurable via CONFIG_HELLO_LED_GPIO_PIN)
 *
 * Build & flash (replace <board> with your target, e.g. esp_wrover_kit/esp32/procpu):
 *   west build -p -b <board> examples/hello_led --build-dir examples/hello_led/build
 *   west flash --build-dir examples/hello_led/build
 */

#include <zephyr/kernel.h>       /* k_msleep, kernel types              */
#include <zephyr/drivers/gpio.h> /* GPIO driver API                     */
#include <zephyr/logging/log.h>  /* Structured logging (LOG_INF, etc.)  */

/* ---------------------------------------------------------------------------
 * Logging Module Registration
 * ---------------------------------------------------------------------------
 * LOG_MODULE_REGISTER creates a named logging module with a default level.
 * The first argument is the module name (appears in log output).
 * The second argument is the default log level:
 *   - LOG_LEVEL_NONE  (0) - No logging
 *   - LOG_LEVEL_ERR   (1) - Only errors
 *   - LOG_LEVEL_WRN   (2) - Errors and warnings
 *   - LOG_LEVEL_INF   (3) - Errors, warnings, and informational messages
 *   - LOG_LEVEL_DBG   (4) - All messages including debug
 */
LOG_MODULE_REGISTER(hello_led, LOG_LEVEL_INF);

/* ---------------------------------------------------------------------------
 * Kconfig-driven Configuration
 * ---------------------------------------------------------------------------
 * These values come from Kconfig (see the Kconfig file in this directory).
 * They are set in prj.conf and become compile-time constants.
 * The CONFIG_ prefix is added automatically by the Kconfig system.
 */
#define BLINK_INTERVAL_MS  CONFIG_HELLO_LED_BLINK_INTERVAL_MS
#define LED_PIN            CONFIG_HELLO_LED_GPIO_PIN

/* GPIO output flags: start with the pin driven HIGH (LED on) */
#define LED_FLAGS          GPIO_OUTPUT_ACTIVE

/* ---------------------------------------------------------------------------
 * main() - Application Entry Point
 * ---------------------------------------------------------------------------
 * In Zephyr, main() runs in the "main thread" which is automatically created
 * by the kernel at boot. This thread has a default priority and stack size
 * defined in the kernel configuration. Unlike bare-metal programs, main()
 * runs within the RTOS scheduler, so calling k_msleep() yields the CPU to
 * other threads (if any) rather than busy-waiting.
 */
int main(void)
{
	int ret;
	bool led_state = true;

	/*
	 * Get the GPIO port device.
	 * DEVICE_DT_GET retrieves a device pointer from the devicetree.
	 * DT_NODELABEL(gpio0) refers to the node labeled "gpio0" in the
	 * board's devicetree. Most boards define gpio0 as the main GPIO port.
	 */
	const struct device *gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));

	if (!device_is_ready(gpio_dev)) {
		LOG_ERR("GPIO device is not ready - check your board configuration");
		return -1;
	}

	/*
	 * Configure the pin as a digital output.
	 * GPIO_OUTPUT_ACTIVE means the pin starts HIGH (LED on).
	 * This function returns 0 on success or a negative error code.
	 */
	ret = gpio_pin_configure(gpio_dev, LED_PIN, LED_FLAGS);
	if (ret < 0) {
		LOG_ERR("Failed to configure GPIO pin %d (error %d)", LED_PIN, ret);
		return ret;
	}

	LOG_INF("Hello LED started - blinking GPIO%d every %d ms",
		LED_PIN, BLINK_INTERVAL_MS);

	/*
	 * Main loop - runs forever.
	 * In an RTOS, the main thread typically never exits. If it did,
	 * the kernel would continue running other threads (if any).
	 */
	while (1) {
		/* Toggle the LED pin (HIGH -> LOW or LOW -> HIGH) */
		gpio_pin_toggle(gpio_dev, LED_PIN);
		led_state = !led_state;

#if CONFIG_HELLO_LED_LOG_ENABLED
		LOG_INF("LED state: %s", led_state ? "ON" : "OFF");
#endif
		/*
		 * k_msleep() puts the current thread to sleep for the given
		 * number of milliseconds. During this time, the CPU is free
		 * to execute other threads. This is fundamentally different
		 * from a busy-wait delay and is a core RTOS concept.
		 */
		k_msleep(BLINK_INTERVAL_MS);
	}

	return 0; /* Never reached, but satisfies the C standard */
}
