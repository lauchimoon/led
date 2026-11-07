CC     ?= gcc
SRC    := led.c
OUT    := led
CFLAGS ?= -g

# Assumes as built from source code
RAYLIB_PATH ?= 
RAYLIB_LIBS := -L$(RAYLIB_PATH)/src -lraylib

INCLUDE = -I$(RAYLIB_PATH)/src
LDLIBS  = $(RAYLIB_LIBS) -lGL -lm -lpthread -ldl -lX11

default: $(OUT)

$(OUT): $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(INCLUDE) $(LDLIBS) -o $(OUT)

.PHONY: default clean
clean:
	rm $(OUT)
