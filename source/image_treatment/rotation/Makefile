CC := gcc
CFLAGS := -Wall -Wextra -g
TARGET = rotate  
LIBFLAGS = -lSDL2 -lSDL2_image -lm

rotate: rotate.c main.c
	$(CC) $(CFLAGS) main.c rotate.c -o rotate $(LIBFLAGS)

clean:
	$(RM) $(TARGET) rotated.png