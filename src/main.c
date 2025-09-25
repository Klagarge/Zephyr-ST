#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <cmsis_os2.h>

#include "led.h"


#define STACK_SIZE CONFIG_CMSIS_V2_THREAD_MAX_STACK_SIZE
static K_THREAD_STACK_ARRAY_DEFINE(stacks, 2, STACK_SIZE);

extern int custom_adder(int a, int b);

static osThreadAttr_t thread_attr[] = {
    {
        .name = "Thread 1",
        .stack_mem = &stacks[0][0],
        .stack_size = STACK_SIZE,
        .priority = osPriorityNormal
    },
    {
        .name = "Thread 2",
        .stack_mem = &stacks[1][0],
        .stack_size = STACK_SIZE,
        .priority = osPriorityNormal
    }
};

osMutexId_t mutex_total;
volatile int total = 0;

void blink(void* led) {
    init_led((struct gpio_dt_spec*)led);

    for(;;) {
        osMutexAcquire (mutex_total, osWaitForever);
        osMutexAcquire (mutex_total, osWaitForever);
        total = custom_adder(total, 1);
        osMutexRelease(mutex_total);
        int ret = gpio_pin_toggle_dt(led);
        if (ret < 0) {
            return;
        }
        osDelay(1000);
    }

}

int main(void) {
    
    mutex_total = osMutexNew(NULL);

	osThreadNew(blink, (void*)&led0, &thread_attr[0]);
    osDelay(500);
	osThreadNew(blink, (void*)&led1, &thread_attr[1]);
    
    
	return 0;
}
