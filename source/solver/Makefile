solver: solver.o main.o
	gcc solver.o main.o -o solver -g -Wall -Wextra

solver.o: solver.c
	gcc -c solver.c -g -Wall -Wextra -o solver.o

main.o: main.c
	gcc -c main.c -g -Wall -Wextra -o main.o

clean:
	rm -f solver solver.o main.o