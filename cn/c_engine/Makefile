# Makefile for NetPulse QoS C Measurement Engine

CC = gcc
CFLAGS = -O2 -Wall -Wextra
WIN_LIBS = -lws2_32 -liphlpapi

TARGET_WIN = netpulse_engine.exe
TARGET_POSIX = netpulse_engine
SRC = netpulse_engine.c

all:
ifeq ($(OS),Windows_NT)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET_WIN) $(WIN_LIBS)
else
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET_POSIX) -lm
endif

clean:
ifeq ($(OS),Windows_NT)
	del $(TARGET_WIN) 2>nul || exit 0
else
	rm -f $(TARGET_POSIX)
endif
