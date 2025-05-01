#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
typedef struct {
    uint16_t x;
    uint16_t y;
} Pixel;

typedef struct {
    Pixel *pixels;
    int nb_pixels;
    int top;
    int bottom;
    int right;
    int left;
} Letter;
