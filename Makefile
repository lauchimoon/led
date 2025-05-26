CC=gcc
SRC=lame.c
OUT=lame
LDLIBS=-Llib/ -lraylib -lGL -lm -lpthread -ldl
INCLUDE=-Iinclude/

default:
	$(CC) $(SRC) $(LDLIBS) $(INCLUDE) -o $(OUT)
