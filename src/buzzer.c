#include <pigpio.h>
#include <pthread.h>
#include "buzzer.h"
#include <wiringPi.h>  

#define BUZZER_GPIO 25
#define TOTAL 32

int notes[] = {
    392,440,523,494,440,392,349,330,
    349,440,392,330,349,294,294,0,
    392,440,523,494,440,392,349,330,
    349,330,294,330,262,262,262,0
};


void* buzzer_thread(void* arg) {
    gpioSetMode(BUZZER_GPIO, PI_OUTPUT);

    for (int i = 0; i < TOTAL; i++) {
        if (notes[i] > 0) {
            // 주파수 설정
            gpioSetPWMfrequency(BUZZER_GPIO, notes[i]);
            // duty 설정 (128 = 50% duty, 볼륨 느낌)
            gpioPWM(BUZZER_GPIO, 128);
        } else {
            // 쉼표 처리
            gpioPWM(BUZZER_GPIO, 0);
        }
        gpioDelay(280 * 1000); // 280ms
    }

    gpioPWM(BUZZER_GPIO, 0); // 끝나면 끔
    pthread_exit(NULL);
}

void buzzer_stop(void) {
    digitalWrite(BUZZER_GPIO, LOW); 
}
