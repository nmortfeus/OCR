#pragma once
#include <SDL2/SDL.h>
#include "Letter.h"

void process(SDL_Surface *surface, Letter **letters, int **grid_letters_index, int *nb_grid_letters, int **list_letters_index, int *nb_list_letters);