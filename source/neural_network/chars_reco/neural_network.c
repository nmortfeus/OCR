#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../../image_treatment/detections/Letter.h"
#include "letters_to_matrices.h"

#define INPUT_SIZE 100
#define HIDDEN_SIZE1 1000
#define HIDDEN_SIZE2 500
#define OUTPUT_SIZE 26

typedef struct {
    double* weights;
    double bias;
    int size;
} Neuron;

double sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

double sigmoid_derivative(double x) {
    double s = sigmoid(x);
    return s * (1.0 - s);
}

void init_neuron(Neuron* neuron, int input_size) {
    neuron->weights = (double*)malloc(input_size * sizeof(double));
    neuron->size = input_size;
    for (int i = 0; i < input_size; i++) {
        neuron->weights[i] = 0.0;
    }
    neuron->bias = 0.0;
}

void load_neurons_from_file(const char* filename, Neuron* hidden_layer1, Neuron* hidden_layer2, Neuron* output_layer) {
    FILE* file = fopen(filename, "r");

    for (int i = 0; i < HIDDEN_SIZE1; i++) {
        fscanf(file, "%lf\n", &hidden_layer1[i].bias);
        hidden_layer1[i].weights = (double*)malloc(INPUT_SIZE * sizeof(double));
        for (int j = 0; j < INPUT_SIZE; j++) {
            fscanf(file, "%lf", &hidden_layer1[i].weights[j]);
        }
        fscanf(file, "\n");
    }

    for (int i = 0; i < HIDDEN_SIZE2; i++) {
        fscanf(file, "%lf\n", &hidden_layer2[i].bias);
        hidden_layer2[i].weights = (double*)malloc(HIDDEN_SIZE1 * sizeof(double));
        for (int j = 0; j < HIDDEN_SIZE1; j++) {
            fscanf(file, "%lf", &hidden_layer2[i].weights[j]);
        }
        fscanf(file, "\n");
    }

    for (int i = 0; i < OUTPUT_SIZE; i++) {
        fscanf(file, "%lf\n", &output_layer[i].bias);
        output_layer[i].weights = (double*)malloc(HIDDEN_SIZE2 * sizeof(double));
        for (int j = 0; j < HIDDEN_SIZE2; j++) {
            fscanf(file, "%lf", &output_layer[i].weights[j]);
        }
        fscanf(file, "\n");
    }

    fclose(file);
}

double compute_output(Neuron* neuron, double* inputs) {
    double sum = neuron->bias;
    for (int i = 0; i < neuron->size; i++) {
        sum += neuron->weights[i] * inputs[i];
    }
    return sigmoid(sum);
}

void forward_prop(Neuron* hidden_layer1, Neuron* hidden_layer2, Neuron* output_layer,
                  double* input_data, double* hidden_outputs1, double* hidden_outputs2, double* output) {

    for (int i = 0; i < HIDDEN_SIZE1; i++) {
        hidden_outputs1[i] = compute_output(&hidden_layer1[i], input_data);
    }

    for (int i = 0; i < HIDDEN_SIZE2; i++) {
        hidden_outputs2[i] = compute_output(&hidden_layer2[i], hidden_outputs1);
    }

    for (int i = 0; i < OUTPUT_SIZE; i++) {
        output[i] = compute_output(&output_layer[i], hidden_outputs2);
    }
}

char max_alphabet(double * output){
    int i = 0;
    for(int j = 1; j < 26; j++){
        if (output[j]>output[i]){
            i = j;
        }
    }
    return 'A' + i;
}

void big_brain(Letter *letters, int *grid_letters_index, int nb_grid_letters, int *list_letters_index, int nb_list_letters, int *col, int *len_words, char ***wordlist, char ***grid){

    
    double **grid_letters_matrices = NULL;
    double **list_letters_matrices = NULL;
    int *words_and_grid_nb = NULL;
    int len_words_etc;

    letters_to_matrices(letters,grid_letters_index,nb_grid_letters,list_letters_index,nb_list_letters,&grid_letters_matrices,&list_letters_matrices,&words_and_grid_nb,&len_words_etc);


    *col = words_and_grid_nb[0];

    Neuron hidden_layer1[HIDDEN_SIZE1];
    Neuron hidden_layer2[HIDDEN_SIZE2];
    Neuron output_layer[OUTPUT_SIZE];

    for (int i = 0; i < HIDDEN_SIZE1; i++) {
        init_neuron(&hidden_layer1[i], INPUT_SIZE);
    }
    for (int i = 0; i < HIDDEN_SIZE2; i++){
        init_neuron(&hidden_layer2[i], HIDDEN_SIZE1);
    }
    for (int i = 0; i < OUTPUT_SIZE; i++){
        init_neuron(&output_layer[i], HIDDEN_SIZE2);
    }
    
    const char* neuron_file = "neurons.txt";
    load_neurons_from_file(neuron_file, hidden_layer1, hidden_layer2, output_layer);
    
    double hidden_outputs1[HIDDEN_SIZE1], hidden_outputs2[HIDDEN_SIZE2], output[OUTPUT_SIZE];
    double **input_data = grid_letters_matrices;
    *grid = malloc((nb_grid_letters/words_and_grid_nb[0])*sizeof(char *));
    char *line = malloc(words_and_grid_nb[0]*sizeof(char));
    int i = 0;
    printf("Grid:\n");
    while (input_data[i] != NULL){
        forward_prop(hidden_layer1, hidden_layer2, output_layer, input_data[i], hidden_outputs1, hidden_outputs2, output);
        char letter = max_alphabet(output);
        if(i == 0 || i%words_and_grid_nb[0] != 0){
            printf("%c",letter);
            line[i%words_and_grid_nb[0]] = letter;
        }
        else{
            (*grid)[(i/words_and_grid_nb[0])-1] = line;
            line = malloc(words_and_grid_nb[0]*sizeof(char));
            printf("\n%c",letter);
            line[0] = letter;
        }
        i++;
    }
    printf("\n");
    (*grid)[nb_grid_letters/words_and_grid_nb[0]-1] = line;
    
    input_data = list_letters_matrices;
    *wordlist = malloc(len_words_etc*sizeof(char *));
    char *word = NULL;
    int len = 0, index = 1;
    i = 0;
    printf("Wordlist:\n");
    while (input_data[i] != NULL){

        forward_prop(hidden_layer1, hidden_layer2, output_layer, input_data[i], hidden_outputs1, hidden_outputs2, output);
        char letter = max_alphabet(output);
        if(i == words_and_grid_nb[index]-1 || input_data[i+1] == NULL){
            word = realloc(word,(len+2)*sizeof(char));
            word[len] = letter;
            word[len+1] = 0;
            (*wordlist)[index-1] = word;
            printf("%s ",word);
            len = 0;
            word = NULL;
            index += 1;
        }
        else{
            word = realloc(word,(len+1)*sizeof(char));
            word[len] = letter;
            len++;
        }
        i++;
    }
    printf("\n");
    (*wordlist)[index-1] = word;
    *len_words = index;
    
    for (int i = 0; i < HIDDEN_SIZE1; i++){
        free(hidden_layer1[i].weights);
        }
    for (int i = 0; i < HIDDEN_SIZE2; i++){
        free(hidden_layer2[i].weights);
    }
    for (int i = 0; i < OUTPUT_SIZE; i++){
        free(output_layer[i].weights);
    }
    
}
