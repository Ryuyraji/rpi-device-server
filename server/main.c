// #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pigpio.h>
#include <dlfcn.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdio.h>

#define PORT 60000
#define BUF_SIZE 1024

static int running = 1;

void sig_handler(int signo) {
    if (signo == SIGINT) running = 0;
}

int main() {
    signal(SIGINT, sig_handler);
    signal(SIGTERM, SIG_IGN);

    if (gpioInitialise() < 0) {
        printf("pigpio 초기화 실패!\n");
        return 1;
    }

    printf("서버 시작 준비...\n");

    // 라이브러리 로드
    void *handle = dlopen("./lib/libdevice.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "dlopen 실패: %s\n", dlerror());
        return 1;
    }

    // 함수 심볼 로드
    void* (*led_thread)(void*)    = dlsym(handle, "led_thread");
    void* (*led_threadpwd)(void*) = dlsym(handle, "led_threadpwd");
    void* (*buzzer_thread)(void*) = dlsym(handle, "buzzer_thread");
    void* (*fnd_thread)(void*)    = dlsym(handle, "fnd_thread");

    // cds 관련 함수 (init/start/stop)
    int  (*cds_init)(void)   = dlsym(handle, "cds_init");
    int  (*cds_start)(void)  = dlsym(handle, "cds_start");
    void (*cds_stop)(void)   = dlsym(handle, "cds_stop");

    // 소켓 준비
    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind 실패");
        return 1;
    }
    if (listen(serv_sock, 5) < 0) {
        perror("listen 실패");
        return 1;
    }

    printf("서버 실행 중... 포트 %d에서 대기 중\n", PORT);

    int clnt_sock = accept(serv_sock, NULL, NULL);
    if (clnt_sock < 0) {
        perror("accept 실패");
        return 1;
    }
    printf("클라이언트 연결됨!\n");

    char buf[BUF_SIZE];

    while (running) {
        memset(buf, 0, BUF_SIZE);
        int len = recv(clnt_sock, buf, BUF_SIZE - 1, 0);
        if (len <= 0) break;
        buf[len] = 0;

        printf("받은 명령: %s\n", buf);

        pthread_t t;
        if (strncmp(buf, "LED ON", 6) == 0) {
            int s = 1;
            pthread_create(&t, NULL, led_thread, &s);
            pthread_detach(t);

        } else if (strncmp(buf, "LED OFF", 7) == 0) {
            int s = 0;
            pthread_create(&t, NULL, led_thread, &s);
            pthread_detach(t);

        } else if (strncmp(buf, "PWM", 3) == 0) {
            char *level = strdup(buf + 4);
            pthread_create(&t, NULL, led_threadpwd, level);
            pthread_detach(t);
            // ⚠ free(level)은 스레드 함수 내부에서 해주는 게 안전합니다

        } else if (strncmp(buf, "BUZZER", 6) == 0) {
            pthread_create(&t, NULL, buzzer_thread, NULL);
            pthread_detach(t);

        } else if (strncmp(buf, "CDS", 3) == 0) {
            if (cds_init && cds_start) {
                if (cds_init() == 0) {
                    if (cds_start() == 0) {
                        printf("CDS 센서 스레드 실행됨\n");
                    } else {
                        printf("CDS 스레드 실행 실패!\n");
                    }
                } else {
                    printf("CDS 초기화 실패!\n");
                }
            } else {
                printf("CDS 함수 심볼을 찾을 수 없음!\n");
            }

        } else if (strncmp(buf, "FND", 3) == 0) {
            int num = atoi(buf + 4);
            int *p = malloc(sizeof(int));
            *p = num;
            pthread_create(&t, NULL, fnd_thread, p);
            pthread_detach(t);

        } else if (strncmp(buf, "EXIT", 4) == 0) {
            printf("클라이언트 종료 요청\n");
            if(cds_stop) cds_stop();
            if (fnd_stop) fnd_stop();
            if (buzzer_stop) buzzer_stop();
            if (led_stop) led_stop();
            break;

        } else {
            printf("알 수 없는 명령: %s\n", buf);
        }
    }

    close(clnt_sock);
    close(serv_sock);

    if (cds_stop) cds_stop();
    dlclose(handle);
    gpioTerminate();

    printf("서버 종료\n");
    return 0;
}
