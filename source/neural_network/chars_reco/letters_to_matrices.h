#pragma once
#include "../../image_treatment/detections/Letter.h"

void letters_to_matrices(Letter *letters, int *grid_letters_index, int nb_grid_letters, int *list_letters_index, int nb_list_letters, double ***grid_letters_matrices, double ***list_letters_matrices,int **words_and_grid_nb, int *len_words_etc);