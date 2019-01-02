CC=i686-w64-mingw32-g++
AR=i686-w64-mingw32-ar
STRIP=i686-w64-mingw32-strip
CFLAGS=-Iinclude -std=c++11 -lurlmon -Os -lwinhttp -static -Wall -Werror
LDFLAGS=-Lbuild -lpayload -lwinhttp -lurlmon

all: dropper main debug

dropper:
	$(CC) -c src/payload.cpp -o build/payload.o $(CFLAGS)
	$(AR) rcs build/libpayload.a build/payload.o

main:
	$(CC) -c src/main.cpp -o build/main.o $(CFLAGS)
	$(CC) -o build/main.dll $(LDFLAGS) -shared
	$(STRIP) -s build/main.dll
	@rm -rf build/*.o
	@echo size of payload: `du -h build/main.dll`

debug:
	$(CC) src/main.cpp -o build/main.exe $(CFLAGS) $(LDFLAGS) -DDEBUG

clean:
	rm -rf build/*
