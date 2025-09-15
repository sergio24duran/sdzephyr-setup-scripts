#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>

#define SLEEP_TIME_MS 500
#define LED_PIN 14
#define LED_FLAGS GPIO_OUTPUT_ACTIVE

void led14_thread(void)
{
    const struct device *gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
    if (!device_is_ready(gpio_dev)) {
        printf("GPIO14 not ready\n");
        return;
    }

    gpio_pin_configure(gpio_dev, LED_PIN, LED_FLAGS);

    bool led_state = true;
    while (1) {
        gpio_pin_toggle(gpio_dev, LED_PIN);
        led_state = !led_state;
        printf("LED14 state: %s\n", led_state ? "ON" : "OFF");
        k_msleep(SLEEP_TIME_MS);
    }
}
