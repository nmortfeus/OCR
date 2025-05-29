#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
        neuron->weights[i] = ((double)rand() / RAND_MAX) - 0.5;
    }
    neuron->bias = ((double)rand() / RAND_MAX) - 0.5;
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

void backpropagation(Neuron* hidden_layer1, Neuron* hidden_layer2, Neuron* output_layer,
                     double* input_data, double* hidden_outputs1, double* hidden_outputs2, double* output,
                     double* expected_output, double learning_rate) {

    double output_errors[OUTPUT_SIZE];
    double hidden_errors2[HIDDEN_SIZE2] = {0};
    double hidden_errors1[HIDDEN_SIZE1] = {0};

    for (int i = 0; i < OUTPUT_SIZE; i++) {
        output_errors[i] = (expected_output[i] - output[i]) * sigmoid_derivative(output[i]);
        for (int j = 0; j < HIDDEN_SIZE2; j++) {
            hidden_errors2[j] += output_errors[i] * output_layer[i].weights[j];
            output_layer[i].weights[j] += learning_rate * output_errors[i] * hidden_outputs2[j];
        }
        output_layer[i].bias += learning_rate * output_errors[i];
    }

    for (int i = 0; i < HIDDEN_SIZE2; i++) {
        hidden_errors2[i] *= sigmoid_derivative(hidden_outputs2[i]);
        for (int j = 0; j < HIDDEN_SIZE1; j++) {
            hidden_errors1[j] += hidden_errors2[i] * hidden_layer2[i].weights[j];
            hidden_layer2[i].weights[j] += learning_rate * hidden_errors2[i] * hidden_outputs1[j];
        }
        hidden_layer2[i].bias += learning_rate * hidden_errors2[i];
    }

    for (int i = 0; i < HIDDEN_SIZE1; i++) {
        hidden_errors1[i] *= sigmoid_derivative(hidden_outputs1[i]);
        for (int j = 0; j < INPUT_SIZE; j++) {
            hidden_layer1[i].weights[j] += learning_rate * hidden_errors1[i] * input_data[j];
        }
        hidden_layer1[i].bias += learning_rate * hidden_errors1[i];
    }
}

void train(int epochs, double learning_rate, double** input_data, double** expected_output,
           int num_samples, Neuron* hidden_layer1, Neuron* hidden_layer2, Neuron* output_layer) {
    for (int epoch = 0; epoch < epochs; epoch++) {
        for (int sample = 0; sample < num_samples; sample++) {
            double hidden_outputs1[HIDDEN_SIZE1];
            double hidden_outputs2[HIDDEN_SIZE2];
            double output[OUTPUT_SIZE];

            forward_prop(hidden_layer1, hidden_layer2, output_layer,
                         input_data[sample], hidden_outputs1, hidden_outputs2, output);
            backpropagation(hidden_layer1, hidden_layer2, output_layer,
                            input_data[sample], hidden_outputs1, hidden_outputs2, output,
                            expected_output[sample], learning_rate);
        }
    }
}

void save_neurons_to_file(const char *filename, Neuron* hidden_layer1, Neuron* hidden_layer2, Neuron* output_layer) {
    FILE *file = fopen(filename, "w");
    for (int i = 0; i < HIDDEN_SIZE1; i++) {
        fprintf(file, "%lf\n",hidden_layer1[i].bias);
        for (int j = 0; j < hidden_layer1[i].size; j++) {
            fprintf(file, "%lf ", hidden_layer1[i].weights[j]);
        }
        fprintf(file,"\n");
    }

    for (int i = 0; i < HIDDEN_SIZE2; i++) {
        fprintf(file, "%lf\n", hidden_layer2[i].bias);
        for (int j = 0; j < hidden_layer2[i].size; j++) {
            fprintf(file, "%lf ", hidden_layer2[i].weights[j]);
        }
        fprintf(file,"\n");
    }

    for (int i = 0; i < OUTPUT_SIZE; i++) {
        fprintf(file, "%lf\n", output_layer[i].bias);
        for (int j = 0; j < output_layer[i].size; j++) {
            fprintf(file, "%lf ", output_layer[i].weights[j]);
        }
        fprintf(file,"\n");
    }

    fclose(file);
}

int read_file(const char *filename, double ***lists, int *num_lists) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return -1;
    }

    char buffer[INPUT_SIZE + 2];
    int capacity = 10;
    *lists = malloc(capacity * sizeof(double *));
    *num_lists = 0;

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        if (*num_lists >= capacity) {
            capacity *= 2;
            double **new_lists = realloc(*lists, capacity * sizeof(double *));
            *lists = new_lists;
        }

        (*lists)[*num_lists] = malloc(INPUT_SIZE * sizeof(double));
        for (int i = 0; i < INPUT_SIZE; i++) {
            if (buffer[i] == '0' || buffer[i] == '1') {
                (*lists)[*num_lists][i] = (double)(buffer[i] - '0');
            } else {
                (*lists)[*num_lists][i] = 0.0;
            }
        }
        (*num_lists)++;
    }

    fclose(file);
    return 0;
}

void letter_to_vector(char letter, double vector[OUTPUT_SIZE]) {
    memset(vector, 0, OUTPUT_SIZE * sizeof(double));
    vector[letter - 'A'] = 1.0;
}

int read_file2(const char *filename, double ***vectors, int *num_vectors) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return -1;
    }

    char buffer[1024];
    *num_vectors = 0;

    if (fgets(buffer, sizeof(buffer), file) != NULL) {
        size_t len = strlen(buffer);
        if (buffer[len - 1] == '\n') buffer[len - 1] = '\0';

        *vectors = malloc(len * sizeof(double*));
        for (size_t i = 0; i < len; i++) {
            (*vectors)[i] = malloc(OUTPUT_SIZE * sizeof(double));
            letter_to_vector(buffer[i], (*vectors)[i]);
            (*num_vectors)++;
        }
    }

    fclose(file);
    return 0;
}


void free_lists(double **lists, int num_lists) {
    for (int i = 0; i < num_lists; i++) {
        free(lists[i]);
    }
    free(lists);
}

int main() {
    int epochs = 1000;
    double learning_rate = 0.1;

    const char *filename = "input.txt";
    double **input_data = NULL;
    int num_lists = 0;
    read_file(filename, &input_data, &num_lists);

    const char *filename2 = "letters.txt";
    double **expected_output = NULL;
    int num_samples = 0;
    read_file2(filename2, &expected_output, &num_samples);

    if(num_lists != num_samples){
        printf("FAILED TO LEARN, DIFFERENT SIZE FROM INPUT AND EXPECTED OUPUT\n");
        return 1;
    }

    Neuron hidden_layer1[HIDDEN_SIZE1];
    Neuron hidden_layer2[HIDDEN_SIZE2];
    Neuron output_layer[OUTPUT_SIZE];

    for (int i = 0; i < HIDDEN_SIZE1; i++){
        init_neuron(&hidden_layer1[i], INPUT_SIZE);
    }
    for (int i = 0; i < HIDDEN_SIZE2; i++){
        init_neuron(&hidden_layer2[i], HIDDEN_SIZE1);
    }
    for (int i = 0; i < OUTPUT_SIZE; i++){
        init_neuron(&output_layer[i], HIDDEN_SIZE2);
    }
    train(epochs, learning_rate, input_data, expected_output, num_samples, hidden_layer1, hidden_layer2, output_layer);

    printf("Results :\n");
    for (int sample = 0; sample < num_samples; sample++) {
        double hidden_outputs1[HIDDEN_SIZE1];
        double hidden_outputs2[HIDDEN_SIZE2];
        double output[OUTPUT_SIZE];

        forward_prop(hidden_layer1, hidden_layer2, output_layer,
                     input_data[sample], hidden_outputs1, hidden_outputs2, output);

        printf("Lettre #%d : ", sample + 1);
        for (int i = 0; i < OUTPUT_SIZE; i++) {
            printf("%.2f ", output[i]);
        }
        printf("\n");
    }

    const char *neuron_file = "neurons.txt";
    save_neurons_to_file(neuron_file, hidden_layer1, hidden_layer2, output_layer);
    printf("Les données des neurones ont été enregistrées dans %s\n", neuron_file);


    free_lists(input_data, num_lists);
    free_lists(expected_output, num_samples);

    return 0;
}
