#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include "letters_finder.h"
#include "Letter.h"

void process(SDL_Surface *surface, Letter **letters, int *nb_letters){
 
    process_letters(surface,letters,nb_letters);
}