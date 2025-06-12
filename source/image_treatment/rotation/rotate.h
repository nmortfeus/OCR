#ifndef ROTATE_H
#define ROTATE_H

#include <err.h>
#include <math.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>





// function to do bilinear interpolation
Uint32 bilinearInterpolation(SDL_Surface* surface, double x, double y);

// function to rotate a sdl surface
SDL_Surface* rotateman(SDL_Surface* surface, double angle);

#endif 
