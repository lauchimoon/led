CC     ?= gcc
SRC    := $(wildcard *.c)
OBJ    := $(SRC:.c=.o)
OUT    := led
CFLAGS ?= -g

# Assumes as built from source code
RAYLIB_PATH ?= 
RAYLIB_LIBS := -L$(RAYLIB_PATH)/src -lraylib

INCLUDE = -I$(RAYLIB_PATH)/src
LDLIBS  = $(RAYLIB_LIBS) -lGL -lm -lpthread -ldl -lX11

DEPS := $(wildcard *.h) fonts/GeistMono-Regular.h

default: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) $(INCLUDE) $(LDLIBS) -o $(OUT)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

.PHONY: default clean
clean:
	rm -f $(OUT) $(OBJ)
