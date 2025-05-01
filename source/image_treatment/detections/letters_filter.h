#pragma once
#include <SDL2/SDL.h>
#include "Letter.h"

void process_letters(SDL_Surface *surface, int x1, int y1, int x2, int y2, Letter **letters, int **grid_letters_index, int *nb_grid_letters, int **list_letters_index, int *nb_list_letters);