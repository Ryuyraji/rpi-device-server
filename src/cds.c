#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <pthread.h>
#include "cds.h"

#define CDS_I2C_ADDR  0x48
#define CDS_CHANNEL   0
#define LED_GPIO      18
#define THRESHOLD     180

static int fd = -1;
static int ledState = LOW;
static pthread_t cds_tid;
static int running = 0;

static void* cds_thread(void* arg) {
    int cnt = 0;
    while (running) {
        // CH0 선택
        wiringPiI2CWrite(fd, 0x00 | CDS_CHANNEL);

        // 첫 번째 값은 Dummy → 버림
        wiringPiI2CRead(fd);
        delay(2);

        // 두 번째 값이 실제 값
        int val = wiringPiI2CRead(fd);

        printf("[CDS %d] a2dVal = %d → %s\n",
               cnt++, val, (val < THRESHOLD) ? "Bright" : "Dark");

       if (val >= THRESHOLD && ledState == LOW) {
    // 어두울 때 LED 켜기
         digitalWrite(LED_GPIO, HIGH);
    ledState = HIGH;
    } else if (val < THRESHOLD && ledState == HIGH) {
    // 밝을 때 LED 끄기
    digitalWrite(LED_GPIO, LOW);
    ledState = LOW;
    return NULL;
    }


        delay(500);
    }
    return NULL;
}

int cds_init(void) {
    if (wiringPiSetupGpio() < 0) {
        perror("wiringPiSetupGpio 실패");
        return -1;
    }

    fd = wiringPiI2CSetupInterface("/dev/i2c-1", CDS_I2C_ADDR);
    if (fd < 0) {
        perror("I2C 초기화 실패");
        return -1;
    }

    pinMode(LED_GPIO, OUTPUT);
    digitalWrite(LED_GPIO, LOW);
    ledState = LOW;
}

int cds_start(void) {
    if (fd < 0) {
        fprintf(stderr, "cds_init() 먼저 호출 필요!\n");
        return -1;
    }
    running = 1;
    if (pthread_create(&cds_tid, NULL, cds_thread, NULL) != 0) {
        perror("pthread_create 실패");
        running = 0;
        return -1;
    }
    return 0;
}

void cds_stop(void) {
    if (running) {
        running = 0;
        pthread_join(cds_tid, NULL);
    }
     digitalWrite(LED_GPIO, LOW);
     ledState = LOW;
}

