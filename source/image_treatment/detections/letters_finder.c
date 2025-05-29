#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <math.h>
#include <stdint.h>
#include "Letter.h"

// Add pixel to a letter
void add_pixel(Letter *letter, int x, int y) {
    Pixel new_pixel = {x, y};
    letter->nb_pixels += 1;
    letter->pixels = realloc(letter->pixels, letter->nb_pixels * sizeof(Pixel));
    letter->pixels[letter->nb_pixels - 1] = new_pixel;
    letter->top = y < letter->top ? y : letter->top;
    letter->bottom = y > letter->bottom ? y : letter->bottom;
    letter->right = x > letter->right ? x : letter->right;
    letter->left = x < letter->left ? x : letter->left;
}

// Get all black pixels around the one at x,y to store them in letter
void add_all_pixels(SDL_Surface *surface, int x, int y, Letter *letter, char *visited) {
    
    int pitch = surface->pitch / sizeof(Uint32);
    Uint32 *surfacePixels = (Uint32 *)surface->pixels;

    if (!surfacePixels) {
        errx(EXIT_FAILURE, "%s", SDL_GetError());
    }

    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h || (Uint8)(surfacePixels[y * pitch + x]) != 0 || visited[y * surface->w + x]) {
        return;
    }

    visited[y * surface->w + x] = 1;
    add_pixel(letter, x, y);
    add_all_pixels(surface, x + 1, y, letter, visited);
    add_all_pixels(surface, x - 1, y, letter, visited);
    add_all_pixels(surface, x, y + 1, letter, visited);
    add_all_pixels(surface, x, y - 1, letter, visited);
}

// Checking if it is a valid letter
int good_letter(Letter letter){
    if (letter.nb_pixels < 30 || letter.nb_pixels > 2000){
        return 0;
    }
    else if (letter.bottom-letter.top > 100 || letter.right - letter.left > 100 || letter.bottom-letter.top < 5){
        return 0;
    }
    else if ((double)(letter.right-letter.left)/(double)(letter.bottom-letter.top) > 10 || (double)(letter.bottom-letter.top)/(double)(letter.right-letter.left) > 10){
        return 0;
    }
    else if ((double)letter.nb_pixels/(double)((letter.bottom-letter.top)*(letter.right-letter.left))<0.1){
        return 0;
    }
    else{
        return 1;
    }
} 


// Detect every possible letters in the image
Letter *find_letters(SDL_Surface *surface,int *nb_letters){
    size_t img_w = surface->w;
    size_t img_h = surface->h;
    int pitch = surface->pitch / sizeof(Uint32);
    Uint32 *surfacePixels = (Uint32 *)surface->pixels;

    if (!surfacePixels) {
        errx(EXIT_FAILURE, "%s", SDL_GetError());
    }

    Letter *letters = NULL;
    char *visited = calloc(img_w * img_h, sizeof(char));
    int nb_letters_return = 0;
    for (int i = 0; i < surface->h; i++) {
        for (int j = 0; j < surface->w; j++) {
            Uint8 p = surfacePixels[i * pitch + j];
            if (p == 0 && !visited[i * img_w + j]) {
                Letter new_letter = {NULL, 0, i, i, j, j};
                add_all_pixels(surface, j, i, &new_letter, visited);
                if (good_letter(new_letter)) {
                    letters = realloc(letters, (nb_letters_return + 1) * sizeof(Letter));
                    letters[nb_letters_return] = new_letter;
                    nb_letters_return += 1;
                    
                } 
                else{
                    free(new_letter.pixels);
                }
            }
        }
    }
    *nb_letters = nb_letters_return;
    free(visited);
    return letters;
}

int min(Letter *letters, int x, int len){
    Letter letter_min = letters[x];
    int min_index = x;
    int i = x + 1;
    while(i < len){
        Letter letter = letters[i];
        if ((letter.top + letter.bottom)/2 < (letter_min.top + letter_min.bottom)/2 - 10 || 
        (abs((letter.top + letter.bottom)/2 - (letter_min.top + letter_min.bottom)/2) < 10 && (letter.right + letter.left)/2 < (letter_min.right + letter_min.left)/2)){
            min_index = i;
            letter_min = letter;
        }
        i++;
    }
    return min_index;
}

// Sort function (to know where letters are on the screen from left to right and top to bottom)
void sort(Letter *letters, int len){
    for(int i = 0; i < len; i++){
        Letter temp_letter = letters[i];
        int min_index = min(letters, i,len);
        letters[i] = letters[min_index];
        letters[min_index] = temp_letter;
    }
}

void process_letters(SDL_Surface *surface, Letter **letters, int *nb_letters){
    *letters = find_letters(surface, nb_letters);
    sort(*letters,*nb_letters); 
}

