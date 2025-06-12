#pragma once
#include <SDL2/SDL.h>
#include "detections/Letter.h"

void display(SDL_Surface *surface, Letter *letters, int *grid_letters_index, int col, int **found);