CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lm

training: neural_training.c
	$(CC) $(CFLAGS) -o training neural_training.c $(LDFLAGS)

clean:
	rm -f training
