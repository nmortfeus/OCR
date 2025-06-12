# Intented for testing purposes only
CC := gcc
CFLAGS := -Wall -Wextra 
TARGET = image_to_bin
TESTFLAGS = -fsanitize=address -g
LIBFLAGS = -lSDL2 -lSDL2_image -lm

image_to_bin: image_to_bin.c main.c
	$(CC) $(CFLAGS) main.c image_to_bin.c -o image_to_bin $(TESTFLAGS) $(LIBFLAGS)

.PHONY: clean
clean:
	$(RM) $(TARGET)
