#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    double weights[2];
    double bias;
} Neurone;

double sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

double sigmoid_derivative(double x) {
    return x * (1.0 - x);
}

void init_neurone(Neurone *neurone) {
    for (int i = 0; i < 2; i++) {
        neurone->weights[i] = ((double)rand()) / RAND_MAX;
    }
    neurone->bias = ((double)rand()) / RAND_MAX;
}

double compute_output(Neurone neurone, double inputs[2]) {
    double sum = inputs[0] * neurone.weights[0] + inputs[1] * neurone.weights[1] + neurone.bias;
    return sigmoid(sum);
}

int main() {
    int epochs = 1000000;
    double learning_rate = 0.1;
    double input_data[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
    double expected_output[4] = {1, 0, 0, 1};
    Neurone hidden_layer[2];
    Neurone output_neurone;
    for (int i = 0; i < 2; i++) {
        init_neurone(&hidden_layer[i]);
    }
    init_neurone(&output_neurone);
    for (int epoch = 0; epoch < epochs; epoch++) {
        for (int i = 0; i < 4; i++) {
            double hidden_layer_output[2];
            for (int j = 0; j < 2; j++) {
                hidden_layer_output[j] = compute_output(hidden_layer[j], input_data[i]);
            }
            double output = compute_output(output_neurone, hidden_layer_output);
            double error = expected_output[i] - output;
            double d_output = error * sigmoid_derivative(output);
            double d_hidden[2];
            for (int j = 0; j < 2; j++) {
                d_hidden[j] = d_output * output_neurone.weights[j] * sigmoid_derivative(hidden_layer_output[j]);
            }
            for (int j = 0; j < 2; j++) {
                output_neurone.weights[j] += hidden_layer_output[j] * d_output * learning_rate;
            }
            output_neurone.bias += d_output * learning_rate;
            for (int j = 0; j < 2; j++) {
                hidden_layer[j].weights[0] += input_data[i][0] * d_hidden[j] * learning_rate;
                hidden_layer[j].weights[1] += input_data[i][1] * d_hidden[j] * learning_rate;
                hidden_layer[j].bias += d_hidden[j] * learning_rate;
            }
        }
    }
    printf("Résultats après l'entraînement :\n");
    for (int i = 0; i < 4; i++) {
        double hidden_layer_output[2];
        for (int j = 0; j < 2; j++) {
            hidden_layer_output[j] = compute_output(hidden_layer[j], input_data[i]);
        }
        double output = compute_output(output_neurone, hidden_layer_output);
        printf("Entrée: [%.1f, %.1f] Sortie attendue: %.1f Sortie obtenue: %.10f\n", input_data[i][0], input_data[i][1], expected_output[i], output);
    }
    return 0;
}