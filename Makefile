ARGS = -Wall -Wextra -g -lSDL2 -lSDL2_image `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -lm 

solver: main.o sdl2gtk.o image_to_bin.o process.o neural_network.o
	gcc main.o sdl2gtk.o image_to_bin.o process.o letters_finder.o neural_network.o letters_to_matrices.o -o solver $(ARGS)

main.o: main.c
	gcc -c main.c $(ARGS)

sdl2gtk.o: source/ui/gtk/sdl2gtk.c
	gcc -c source/ui/gtk/sdl2gtk.c $(ARGS)

image_to_bin.o: source/image_treatment/image_to_bin/image_to_bin.c
	gcc -c source/image_treatment/image_to_bin/image_to_bin.c $(ARGS)

process.o: source/image_treatment/detections/process.c letters_finder.o
	gcc -c source/image_treatment/detections/process.c $(ARGS)

letters_finder.o: source/image_treatment/detections/letters_finder.c
	gcc -c source/image_treatment/detections/letters_finder.c $(ARGS)

letters_to_matrices.o: source/neural_network/letters_to_matrices.c
	gcc -c source/neural_network/letters_to_matrices.c $(ARGS)

neural_network.o: source/neural_network/neural_network.c letters_to_matrices.o
	gcc -c source/neural_network/neural_network.c $(ARGS)


run: solver
	./solver

clean:
	rm -f *.o solver