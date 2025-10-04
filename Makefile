CC = gcc
CFLAGS = -Wall -Iinclude -fPIC
LIBS = -lpigpio -lpthread -ldl -lwiringPi

all: lib/libdevice.so server_app

# 공유 라이브러리 빌드
lib/libdevice.so: src/led.o src/buzzer.o src/cds.o src/fnd.o
	$(CC) -shared -o $@ $^ $(LIBS)

# 개별 오브젝트 파일 빌드 규칙
src/%.o: src/%.c include/%.h
	$(CC) $(CFLAGS) -c $< -o $@

# 서버 빌드 (실행파일 이름 변경)
server_app: server/main.c
	$(CC) -o $@ $^ -ldl $(LIBS)

# 청소
clean:
	rm -f src/*.o lib/*.so server_app

