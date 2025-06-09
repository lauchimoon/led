CC=gcc
SRC=led.c
OUT=led
LDLIBS=-Llib/ -lraylib -lGL -lm -lpthread -ldl
INCLUDE=-Iinclude/
CFLAGS=-g

default:
	$(CC) $(SRC) $(LDLIBS) $(INCLUDE) $(CFLAGS) -o $(OUT)
