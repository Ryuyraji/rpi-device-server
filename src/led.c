#include <pigpio.h>
#include <pthread.h>
#include <string.h>
#include "led.h"

#define LED_GPIO 18

void* led_thread(void* arg) {
    int state = *((int*)arg);
    gpioSetMode(LED_GPIO, PI_OUTPUT);
    gpioWrite(LED_GPIO, state ? 1 : 0);
    pthread_exit(NULL);
}

void* led_threadpwd(void* arg) {
    const char* level = (const char*)arg;
    int value = 0;
    if (strcmp(level, "high") == 0) value = 255;
    else if (strcmp(level, "middle") == 0) value = 128;
    else if (strcmp(level, "low") == 0) value = 64;
    else pthread_exit(NULL);

    gpioSetMode(LED_GPIO, PI_OUTPUT);
    gpioPWM(LED_GPIO, value);
    pthread_exit(NULL);
}

