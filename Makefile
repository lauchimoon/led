CC=gcc
SRC=lame.c
OUT=lame
LDLIBS=-Llib/ -lraylib -lGL -lm -lpthread -ldl
INCLUDE=-Iinclude/
CFLAGS=-g

default:
	$(CC) $(SRC) $(LDLIBS) $(INCLUDE) $(CFLAGS) -o $(OUT)
