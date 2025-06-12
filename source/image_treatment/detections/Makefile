CC := gcc
CFLAGS := -Wall -Wextra 
TARGET = detection
LIBFLAGS = -lSDL2 -lSDL2_image -lm

detection: main.c process.c ../image_to_bin/image_to_bin.c hough_transform.c letters_filter.c letters_finder.c ../rotation/rotate.c letters_to_matrices.c
	$(CC) $(CFLAGS) main.c process.c ../image_to_bin/image_to_bin.c hough_transform.c letters_filter.c ../rotation/rotate.c letters_finder.c -o detection letters_to_matrices.c $(LIBFLAGS)

.PHONY: clean
clean:
	$(RM) $(TARGET)
