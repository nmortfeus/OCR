#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include "hough_transform.h"
#include "letters_filter.h"
#include "Letter.h"

void process(SDL_Surface *surface, Letter **letters, int **grid_letters_index, int *nb_grid_letters, int **list_letters_index, int *nb_list_letters){
    
    int x1,x2,y1,y2;
    process_hough(surface,&x1,&x2,&y1,&y2);
    process_letters(surface,x1,y1,x2,y2,letters,grid_letters_index,nb_grid_letters,list_letters_index,nb_list_letters);
}