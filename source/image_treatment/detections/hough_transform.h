#pragma once
#include <SDL2/SDL.h>

extern const double pi;
void process_hough(SDL_Surface *surface, int *x1, int*x2, int *y1, int *y2);
int *hough_transform(SDL_Surface *surface, int *to_return_size);
Uint8 *canny_edge_detector(SDL_Surface *surface, int img_w, int img_h);