/*
 * Blinky on ESP32 DevKitC using GPIO14
 * Zephyr RTOS
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>

/* Blink time in ms */
#define SLEEP_TIME_MS 1000

/* Physical LED pin */
#define LED_PIN 14
#define LED_FLAGS GPIO_OUTPUT_ACTIVE

int main(void)
{
    int ret;
    bool led_state = true;

    /* Get the GPIO0 device of the ESP32 (all pins are here) */
    const struct device *gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
    if (!device_is_ready(gpio_dev)) {
        printf("Error: GPIO not ready\n");
        return -1;  // Indicate error, although it will never return in RTOS
    }

    /* Configure the pin as active output */
    ret = gpio_pin_configure(gpio_dev, LED_PIN, LED_FLAGS);
    if (ret < 0) {
        printf("Error configuring GPIO%d\n", LED_PIN);
        return ret;
    }

    while (1) {
        gpio_pin_toggle(gpio_dev, LED_PIN);
        led_state = !led_state;
        printf("LED state: %s\n", led_state ? "ON" : "OFF");
        k_msleep(SLEEP_TIME_MS);
    }

    return 0; // Never reached, but complies with C standard
}
