#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <math.h>
#include <stdint.h>
#include "../../image_treatment/detections/Letter.h"

int MATRIX_SIZE = 10;

void print_matrix(double *matrix) {
    printf("------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            printf("%i", (int)matrix[i * MATRIX_SIZE + j]);
        }
        printf("\n");
    }
    printf("\n------------------------------------------------------------------------------------------\n");
}

double is_black(Letter letter, int x, int y){
    int i = 0;
    while(i < letter.nb_pixels && (letter.pixels[i].x-letter.left != x || letter.pixels[i].y-letter.top != y)){
        i++;
    }
    return (double)(i < letter.nb_pixels);
}

void write_list_to_file(const char *filename, int *list, int size) {
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    for (int i = 0; i < size; i++) {
        fprintf(file, "%d", list[i]);
    }
    fprintf(file, "\n"); 
    fclose(file);
}

void letters_to_matrices(Letter *letters, int *grid_letters_index, int nb_grid_letters, int *list_letters_index, int nb_list_letters, double ***grid_letters_matrices, double ***list_letters_matrices, int **words_and_grid_nb, int *len_words_etc) {
    *len_words_etc = 0;
    *grid_letters_matrices = malloc((nb_grid_letters + 1)* sizeof(double *));
    int midy = (letters[grid_letters_index[0]].bottom + letters[grid_letters_index[0]].top) / 2;
    for (int i = 0; i < nb_grid_letters; i++) {
        Letter letter = letters[grid_letters_index[i]];
        int letter_midy = (letter.bottom + letter.top) / 2;
        // How many letters on one line in the grid
        if(midy != -1 && abs(letter_midy - midy)>10){
            *words_and_grid_nb = realloc(*words_and_grid_nb,(*len_words_etc+1)*sizeof(int));
            (*words_and_grid_nb)[*len_words_etc] = i;
            *len_words_etc += 1;
            midy = -1;
        }
        float height = letter.bottom-letter.top;
        float width = letter.right-letter.left;
        double *matrix = calloc(MATRIX_SIZE * MATRIX_SIZE, sizeof(double));
        for(int j = 0; j < MATRIX_SIZE; j ++){
            for(int k = 0; k < MATRIX_SIZE; k++){
                float ratio_j = (float)j/(float)MATRIX_SIZE;
                float ratio_k = (float)k/(float)MATRIX_SIZE;
                matrix[j * MATRIX_SIZE + k] = is_black(letter,(int)(ratio_k*width),(int)(ratio_j*height));
            }
        }
        (*grid_letters_matrices)[i] = matrix;
    }
    (*grid_letters_matrices)[nb_grid_letters] = (double *)NULL;

    
    *list_letters_matrices = malloc((nb_list_letters + 1) * sizeof(double *));
    midy = (letters[list_letters_index[0]].bottom + letters[list_letters_index[0]].top) / 2;
    int rightx = letters[list_letters_index[0]].right;
    for (int i = 0; i < nb_list_letters; i++) {
        Letter letter = letters[list_letters_index[i]];
        int letter_midy = (letter.bottom + letter.top) / 2;
        if(i != 0 && (abs(letter_midy - midy)>10 || abs(letter.left - rightx)>10)){
            *words_and_grid_nb = realloc(*words_and_grid_nb,(*len_words_etc+1)*sizeof(int));
            (*words_and_grid_nb)[*len_words_etc] = i;
            *len_words_etc += 1;
        }
        rightx = letter.right;
        midy = letter_midy;
        float height = letter.bottom-letter.top;
        float width = letter.right-letter.left;
        double *matrix = calloc(MATRIX_SIZE * MATRIX_SIZE, sizeof(double));
        for(int j = 0; j < MATRIX_SIZE; j ++){
            for(int k = 0; k < MATRIX_SIZE; k++){
                float ratio_j = (float)j/(float)MATRIX_SIZE;
                float ratio_k = (float)k/(float)MATRIX_SIZE;
                matrix[j * MATRIX_SIZE + k] = is_black(letter,(int)(ratio_k*width),(int)(ratio_j*height));
            }
        }
        (*list_letters_matrices)[i] = matrix;
    }
    (*list_letters_matrices)[nb_list_letters] = (double *)NULL;
    

}