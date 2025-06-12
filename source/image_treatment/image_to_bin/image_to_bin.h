#pragma once
#include <SDL2/SDL.h>

// For testing purposes
void gaussian_blur(SDL_Surface* surface);
void to_grayscale(SDL_Surface* surface, unsigned char* image, int w, int h);
void integral_image(int w, int h, unsigned char* src, unsigned long* dest, unsigned long* sq_dest);
void apply_contrast(unsigned char* image, int w, int h, unsigned long* integral);
void sauvola(int w, int h, unsigned long* integral, unsigned long* sq_integral, unsigned char* image);
void cleanup_noise(unsigned char* image, int w, int h);
void change_surface(SDL_Surface* surface, unsigned char* image, int w, int h);
// ---

void image_to_bin(SDL_Surface* surface);