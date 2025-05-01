ARGS = -Wall -Wextra -g -lSDL2 -lSDL2_image `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -lm 

solver: main.o sdl2gtk.o image_to_bin.o process.o neural_network.o display.o
	gcc main.o sdl2gtk.o image_to_bin.o process.o hough_transform.o letters_filter.o letters_finder.o rotate.o neural_network.o letters_to_matrices.o solver.o display.o -o solver $(ARGS)

main.o: main.c
	gcc -c main.c $(ARGS)

sdl2gtk.o: sdl2gtk.c
	gcc -c sdl2gtk.c $(ARGS)

image_to_bin.o: ../../image_treatment/image_to_bin/image_to_bin.c
	gcc -c ../../image_treatment/image_to_bin/image_to_bin.c $(ARGS)

process.o: ../../image_treatment/detections/process.c hough_transform.o letters_filter.o letters_finder.o
	gcc -c ../../image_treatment/detections/process.c $(ARGS)

hough_transform.o: ../../image_treatment/detections/hough_transform.c rotate.o
	gcc -c ../../image_treatment/detections/hough_transform.c $(ARGS)

letters_filter.o: ../../image_treatment/detections/letters_filter.c
	gcc -c ../../image_treatment/detections/letters_filter.c $(ARGS)

letters_finder.o: ../../image_treatment/detections/letters_finder.c
	gcc -c ../../image_treatment/detections/letters_finder.c $(ARGS)

rotate.o: ../../image_treatment/rotation/rotate.c
	gcc -c ../../image_treatment/rotation/rotate.c $(ARGS)

letters_to_matrices.o: ../../neural_network/chars_reco/letters_to_matrices.c
	gcc -c ../../neural_network/chars_reco/letters_to_matrices.c $(ARGS)

neural_network.o: ../../neural_network/chars_reco/neural_network.c letters_to_matrices.o solver.o
	gcc -c ../../neural_network/chars_reco/neural_network.c $(ARGS)

solver.o: ../../solver/solver.c
	gcc -c ../../solver/solver.c $(ARGS)

display.o: ../../image_treatment/display.c
	gcc -c ../../image_treatment/display.c $(ARGS)

run: solver
	./solver

clean:
	rm -f *.o solver