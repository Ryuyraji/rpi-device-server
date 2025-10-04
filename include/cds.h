#ifndef CDS_H
#define CDS_H

// CDS 센서 쓰레드 함수 (내부적으로 사용 가능)

// 라이브러리에서 제공할 API
int cds_init(void);   // 초기화
int cds_start(void);  // 스레드 시작
void cds_stop(void);  // 스레드 정지

#endif

