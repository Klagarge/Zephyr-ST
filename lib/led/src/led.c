#include "led.h"

int init_led(const struct gpio_dt_spec* led) {
    int ret;

    if (!gpio_is_ready_dt(led)) {
        return 0;
    }

    ret = gpio_pin_configure_dt(led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return 0;
    }
}
