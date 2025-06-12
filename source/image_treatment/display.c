#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <math.h>
#include <stdint.h>
#include "Letter.h"

void draw_line_finish(SDL_Surface *surface, int x1, int y1, int x2, int y2, Uint32 color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    int pitch = surface->pitch / sizeof(Uint32);
    Uint32 *pixels = (Uint32 *)surface->pixels;

    int line_thickness = 1;

    while (1) {
        if (x1 >= 0 && y1 >= 0 && x1 < surface->w && y1 < surface->h) {
            for (int dx_offset = -line_thickness; dx_offset <= line_thickness; dx_offset++) {
                for (int dy_offset = -line_thickness; dy_offset <= line_thickness; dy_offset++) {
                    int new_x = x1 + dx_offset;
                    int new_y = y1 + dy_offset;

                    if (new_x >= 0 && new_y >= 0 && new_x < surface->w && new_y < surface->h) {
                        pixels[new_y * pitch + new_x] = color;
                    }
                }
            }
        }

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void display(SDL_Surface *surface, Letter *letters, int *grid_letters_index, int col, int **found){
    int i = 0;
    Uint32 color = SDL_MapRGB(surface->format, 0, 0, 0);
    while(found[i] != NULL){
        Letter letter1 = letters[grid_letters_index[found[i][1] * col + found[i][0]]];
        Letter letter2 = letters[grid_letters_index[found[i][3] * col + found[i][2]]];
        draw_line_finish(surface, (letter1.right + letter1.left) / 2, 
            (letter1.bottom + letter1.top) / 2, 
            (letter2.right + letter2.left) / 2, 
            (letter2.bottom + letter2.top) / 2, 
            color);
        i++;
    }
}
