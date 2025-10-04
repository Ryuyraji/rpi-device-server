#include <pigpio.h>
#include <pthread.h>
#include <unistd.h>
#include "fnd.h"
#include "buzzer.h"

void* fnd_thread(void* arg) {
    int gpiopins[4] = {16,20,21,12};
    int number[10][4] = {
        {0,0,0,0}, {0,0,0,1}, {0,0,1,0}, {0,0,1,1},
        {0,1,0,0}, {0,1,0,1}, {0,1,1,0}, {0,1,1,1},
        {1,0,0,0}, {1,0,0,1}
    };
    int num = *((int*)arg);
    for (int i=0; i<4; i++) gpioSetMode(gpiopins[i], PI_OUTPUT);
    while (num >= 0) {
        for (int i=0; i<4; i++)
            gpioWrite(gpiopins[i], number[num][i]?1:0);
        sleep(1);
        if (num == 0) {
            buzzer_thread(NULL);
        }
        num--;
    }
    for (int i=0; i<4; i++) gpioWrite(gpiopins[i], 1);
    pthread_exit(NULL);
}

