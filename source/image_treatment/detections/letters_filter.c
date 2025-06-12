#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <math.h>
#include <stdint.h>
#include "letters_finder.h"
#include "Letter.h"

// For test
#include "letters_to_matrices.h"

// Display a letter to the screen
void draw_letter(SDL_Surface *surface, Letter letter, int c){
    Pixel *pixels = letter.pixels;
    Uint32 color = SDL_MapRGB(surface->format, 255*c, 0, abs(c-1)*255);
    int pitch = surface->pitch / sizeof(Uint32);
    for (int i = 0; i < letter.nb_pixels; i++){
        Pixel pixel = pixels[i];
        ((Uint32*)surface->pixels)[pixel.y * pitch + pixel.x] = color;
    }
}

// Display every letters on screen
void draw_every_letters(SDL_Surface* surface, Letter *letters, int *grid_letters_index, int nb_grid_letters, int *list_letters_index, int nb_list_letters){
    int i = 0;
    while (i<nb_grid_letters || i<nb_list_letters){
        if (i < nb_grid_letters){
            draw_letter(surface,letters[grid_letters_index[i]],1);
        }
        if (i < nb_list_letters){
            draw_letter(surface,letters[list_letters_index[i]],0);
        }
        i++;
    }
}

// Draw a line on screen
void draw_line(SDL_Surface *surface, int x1, int y1, int x2, int y2, Uint32 color) {
    int pitch = surface->pitch / sizeof(Uint32);
    if (x1==x2){
        for (int i = y1; i < y2; i++){
            ((Uint32*)surface->pixels)[i * pitch + x1] = color;
        }
    }
    else{
        for (int i = x1; i < x2; i++){
            ((Uint32*)surface->pixels)[y1 * pitch + i] = color;
        }
    }
}

// Display a square on screen
void draw_square(SDL_Surface *surface, int x1, int y1, int x2, int y2, int c) {
    Uint32 color = SDL_MapRGB(surface->format, 255 * c, 0, abs(c - 1) * 255);

    draw_line(surface, x1, y1, x2, y1, color);
    draw_line(surface, x1, y2, x2, y2, color);
    draw_line(surface, x1, y1, x1, y2, color);
    draw_line(surface, x2, y1, x2, y2, color);
}

// Draw on screen the grid and list of letters
void draw_grid_list(SDL_Surface *surface, int x1_g, int y1_g, int x2_g, int y2_g, int x1_l, int y1_l, int x2_l, int y2_l){
    if (x1_g == -1){
        return;
    }
    else if (x1_l == -1){
        draw_square(surface,x1_g,y1_g,x2_g,y2_g,1);
    }
    else{
        draw_square(surface,x1_l,y1_l,x2_l,y2_l,0);
        draw_square(surface,x1_g,y1_g,x2_g,y2_g,1);
    }
}

// For test
void print_array(int *array, int len){
    printf("[ ");
    for (int i = 0; i < len; i++){
        printf("%i ",array[i]);
    }
    printf("]\n");
}

// Add the axis close to the middle of the letter at index in the accumulator
void add_accumulator(Letter *letters, int index, int *x_axis, int *y_axis){
    Letter letter = letters[index];
    int x = (letter.right + letter.left) / 2;
    int y = (letter.bottom + letter.top) / 2;
    for(int i = -3; i < 4; i++){
        x_axis[x+i] += 1;
        y_axis[y+i] += 1;
    }
}

// Aux function to the min distance method 
int is_grid_letter_average(Letter *letters, double *average, int index, int average_value){
    if(average[index] < average_value){ 
        return 0;
    }
    // Grid letter can't be connected because of bad grayscale since they are far from each others
    else if(((double)letters[index].bottom-letters[index].top)/(double)(letters[index].right-letters[index].left)<0.5){
        return 0;
    }
    else{
        return 1;
    }
}

// Aux function to the axis method
int is_grid_letter_axis(Letter *letters, int *x_axis, int *y_axis, int index){
    if(x_axis[(letters[index].right + letters[index].left)/2]<8 || y_axis[(letters[index].bottom + letters[index].top)/2]<8){
        return 0;
    }
    else{
        return 1;
    }
}

// Find grid letters using the minimal distance method
void find_by_min_distance(Letter *letters, int nb_letters, int **grid_letters_index, int *nb_grid_letters, int **list_letters_index, int *nb_list_letters){
    double *average = calloc(nb_letters,sizeof(double));
    double average_value = 0.0;
    for (int i = 0; i < nb_letters; i++){
        if (average[i]){
            continue;
        }
        Letter letter = letters[i];
        double center_x = (letter.right + letter.left) / 2.0;
        double center_y = (letter.bottom + letter.top) / 2.0;
        double min = -1;
        int index = 0;
        for (int j = 0 ; j < nb_letters; j++){
            Letter letter2 = letters[j];
            double center2_x = (letter2.right + letter2.left) / 2.0;
            double center2_y = (letter2.bottom + letter2.top) / 2.0;
            double distance = sqrt((double)((center_y-center2_y)*(center_y-center2_y)+(center_x-center2_x)*(center_x-center2_x)));
            if (j!=i && (min == -1 || distance < min)){
                min = distance;
                index = j;
            }
        }
        average[i] = min;
        average[index] = min;
    }
    for(int i = 0; i < nb_letters; i++){
        average_value += average[i];
    }
    average_value /= nb_letters;
    for (int i = 0; i < nb_letters; i++){
        if (is_grid_letter_average(letters,average,i,average_value)){
            *grid_letters_index = realloc(*grid_letters_index,(*nb_grid_letters+1)*sizeof(int));
            (*grid_letters_index)[*nb_grid_letters] = i;
            *nb_grid_letters+=1;
        }
        else{
            *list_letters_index = realloc(*list_letters_index,(*nb_list_letters+1)*sizeof(int));
            (*list_letters_index)[*nb_list_letters] = i;
            *nb_list_letters+=1;
        }
    }
    free(average);
} 

// Shift the element at index to the end of the array
void shift(int *array, int index, int len){
    int i = index;
    while (i < len-1){
        int temp = array[i+1];
        array[i+1] = array[i];
        array[i] = temp;
        i++;
    }
}

// Find grid letters using the axis accumulator method
void find_by_axis(Letter *letters, int **grid_letters_index, int *nb_grid_letters, int **list_letters_index, int *nb_list_letters,int img_w, int img_h){
    int *x_axis = calloc(img_w,sizeof(int));
    int *y_axis = calloc(img_h,sizeof(int));
    for (int i = 0; i < *nb_grid_letters; i++){
        add_accumulator(letters,(*grid_letters_index)[i],x_axis,y_axis);
    }
    for(int i = *nb_grid_letters-1; i >=0; i--){
        if (!is_grid_letter_axis(letters,x_axis,y_axis,(*grid_letters_index)[i])){
            *list_letters_index = realloc(*list_letters_index,(*nb_list_letters+1)*sizeof(int));
            (*list_letters_index)[*nb_list_letters] = (*grid_letters_index)[i];
            *nb_list_letters+=1;
            shift(*grid_letters_index,i,*nb_grid_letters);
            *grid_letters_index = realloc(*grid_letters_index,(*nb_grid_letters-1)*sizeof(int));
            *nb_grid_letters-=1;
        }
    }
    free(x_axis);
    free(y_axis);
}

// If hough didn't work, find the grid and the list
void find_grid_and_list(Letter *letters, int nb_letters, int **grid_letters_index, int *nb_grid_letters, int **list_letters_index, int *nb_list_letters, int img_w, int img_h){
    find_by_min_distance(letters,nb_letters,grid_letters_index,nb_grid_letters,list_letters_index,nb_list_letters);
    find_by_axis(letters,grid_letters_index,nb_grid_letters,list_letters_index,nb_list_letters,img_w,img_h); 
}

// Making the letter at index a grid letter
void make_grid_letter(int **grid_letters_index, int *nb_grid_letters, int **list_letters_index, int *nb_list_letters, int index){
    for (int i = 0; i < *nb_grid_letters; i++){
        if ((*grid_letters_index)[i] == index){
            return;
        }
    }
    *grid_letters_index = realloc(*grid_letters_index,((*nb_grid_letters+1)*sizeof(int)));
    (*grid_letters_index)[*nb_grid_letters] = index;
    *nb_grid_letters += 1;
    for(int i = 0; i < *nb_list_letters; i++){
        if ((*list_letters_index)[i] == index){
            shift(*list_letters_index,i,*nb_list_letters);
            *list_letters_index = realloc(*list_letters_index,(*nb_list_letters-1)*sizeof(int));
            *nb_list_letters-=1;
            return;
        }
    }
}

// If hough worked, every letters inside the grid is a letter of the grid
void grid_detected(Letter *letters, int nb_letters, int **grid_letters_index, int *nb_grid_letters, int **list_letters_index, int *nb_list_letters, int x1, int y1, int x2, int y2){
    for(int i = 0; i < nb_letters; i++){
        Letter letter = letters[i];
        double center_x = (letter.right + letter.left) / 2.0;
        double center_y = (letter.bottom + letter.top) / 2.0;
        if (center_x < x2 && center_x > x1 && center_y < y2 && center_y > y1){
            make_grid_letter(grid_letters_index,nb_grid_letters,list_letters_index,nb_list_letters,i);
        }
    }
}

// Get coordinates of the square where letters are in
void get_coordinates(Letter *letters, int *letters_index, int nb_letters, int *x1, int *y1, int *x2, int *y2){
    for (int i = 0; i < nb_letters; i++){
        Letter letter = letters[letters_index[i]];
        if (*x1 == -1 || letter.left < *x1){
            *x1 = letter.left;
        }
        if (*y1 == -1 || letter.top < *y1){
            *y1 = letter.top;
        }
        if (*x2 == -1 || letter.right > *x2){
            *x2 = letter.right;
        }
        if (*y2 == -1 || letter.bottom > *y2){
            *y2 = letter.bottom;
        }
    }
}

// Check if more letters are bellow, at right, at left or at the top of the grid and keep only word list lettes
void find_list_by_placement(Letter *letters, int **list_letters_index, int *nb_list_letters, int x1, int y1, int x2, int y2)
{
    int top = 0, bottom = 0, right = 0, left = 0;
    for (int i = 0; i < *nb_list_letters; i++){
        Letter letter = letters[(*list_letters_index)[i]];
        double center_x = (letter.right + letter.left) / 2.0;
        double center_y = (letter.bottom + letter.top) / 2.0;
        // Divided by 2 because the number of letters is more important than the numbers of pixels
        if(center_x < x1){
            left+=letter.nb_pixels/2;
        }
        else if(center_x > x2){
            right+=letter.nb_pixels/2;
        }
        if (center_y < y1){
            top+=letter.nb_pixels/2;
        }
        else if (center_y > y2){
            bottom += letter.nb_pixels/2;
        }
    }
    int max = top;
    int region = 0;
    
    if (bottom > max) { 
        max = bottom; region = 1; 
        }
    if (right > max) {
        max = right; region = 2;
        }
    if (left > max) {
        max = left; region = 3;
    }
    for (int i = 0; i < *nb_list_letters; i++) {
        Letter letter = letters[(*list_letters_index)[i]];
        double center_x = (letter.right + letter.left) / 2.0;
        double center_y = (letter.bottom + letter.top) / 2.0;
        
        // Check for the placement
        if ((region == 0 && center_y > y2) || (region == 1 && center_y < y2) || (region == 2 && center_x < x2) || (region == 3 && center_x > x1)) {
            shift(*list_letters_index,i,*nb_list_letters);
            *list_letters_index = realloc(*list_letters_index,(*nb_list_letters-1)*sizeof(int));
            *nb_list_letters -= 1;
            i--;
        }
    }

}

// Find the list letters from noises by average height
void find_list_by_height(Letter *letters, int **list_letters_index, int *nb_list_letters){
    int *all_height = calloc(50,sizeof(int));
    for(int i = 0; i < *nb_list_letters; i++){
        Letter letter = letters[(*list_letters_index)[i]];
        int size = letter.bottom - letter.top;
        for(int k = -2; k < 3; k++){
            if(size+k < 50 && size+k > 0){ 
                all_height[size+k]++;
            }
        }
    }
    int peak_height_index = -1;
    for(int i = 0; i < 50; i++){
        if (peak_height_index == -1 || all_height[i] > all_height[peak_height_index]){
            peak_height_index = i;
        }
    }
    for(int i = 0; i < *nb_list_letters; i++){
        // The letter having a size too much different than the peak is a noise
        if(abs(letters[(*list_letters_index)[i]].bottom - letters[(*list_letters_index)[i]].top - peak_height_index)>5){
            shift(*list_letters_index,i,*nb_list_letters);
            *list_letters_index = realloc(*list_letters_index,(*nb_list_letters-1)*sizeof(int));
            *nb_list_letters -= 1;
            i--;
        }
    }
    free(all_height);
}

// Delete noise from word list letters
void list_letters_or_not(Letter *letters, int **list_letters_index, int *nb_list_letters, int x1, int y1, int x2, int y2){
    find_list_by_placement(letters,list_letters_index,nb_list_letters,x1,y1,x2,y2);
    find_list_by_height(letters,list_letters_index,nb_list_letters);
}

int min(Letter *letters, int *array_index, int len){
    int min_index = 0;
    Letter letter_min = letters[array_index[min_index]];
    int i = 0;
    while(i < len){
        Letter letter = letters[array_index[i]];
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
void sort(Letter *letters, int *array_index, int len){
    for(int i = 0; i < len; i++){
        int temp_index = array_index[i];
        int min_index = min(letters, array_index+i,len-i);
        array_index[i] = array_index[min_index+i];
        array_index[min_index+i] = temp_index;
    }
}

// Free everything
void free_all(Letter *letters, int nb_letters, int *grid_letters_index, int *list_letters_index){
    free(grid_letters_index);
    free(list_letters_index);
    for (int i = 0; i < nb_letters; i++) {
        free(letters[i].pixels);
    }
    free(letters);
}

/* Main Function */
// First step is to find all letters, second step is to separate the grid letters and the word list letters, last step is to delete noises from word list letters
void process_letters(SDL_Surface *surface, int x1, int y1, int x2, int y2, Letter **letters, int **grid_letters_index, int *nb_grid_letters, int **list_letters_index, int *nb_list_letters){

    // First step
    int nb_letters;
    *letters = find_letters(surface, &nb_letters);

    // Function to separate the grid and the list (second step)
    find_grid_and_list(*letters, nb_letters, grid_letters_index, nb_grid_letters, list_letters_index, nb_list_letters,surface->w,surface->h);

    // If a grid is detected then inside the grid there is for sure grid letters ! (second step)
    if (!(x1==0 && y1==0 && x2==surface->w && y2==surface->h)){
        grid_detected(*letters, nb_letters, grid_letters_index, nb_grid_letters, list_letters_index, nb_list_letters, x1, y1, x2, y2);
    }

    int x1_g=-1,y1_g=-1,x2_g=-1,y2_g=-1;
    get_coordinates(*letters,*grid_letters_index,*nb_grid_letters,&x1_g,&y1_g,&x2_g,&y2_g);

    // Delete noises from letters list (last step)
    list_letters_or_not(*letters,list_letters_index,nb_list_letters,x1_g,y1_g,x2_g,y2_g);

    int x1_l=-1,y1_l=-1,x2_l=-1,y2_l=-1;
    get_coordinates(*letters,*list_letters_index,*nb_list_letters,&x1_l,&y1_l,&x2_l,&y2_l);
    
    draw_every_letters(surface,*letters,*grid_letters_index,*nb_grid_letters,*list_letters_index,*nb_list_letters);

    printf("Grid detection results: x1=%i y1=%i x2=%i y2=%i\n",x1_g,y1_g,x2_g,y2_g);
    printf("List detection results: x1=%i y1=%i x2=%i y2=%i\n",x1_l,y1_l,x2_l,y2_l);

    // Display the grid and list squares on screen
    draw_grid_list(surface,x1_g,y1_g,x2_g,y2_g,x1_l,y1_l,x2_l,y2_l);

    sort(*letters,*grid_letters_index,*nb_grid_letters);
    sort(*letters,*list_letters_index,*nb_list_letters);

    
    
    /*
    int **grid_letters_matrices = NULL;
    int **list_letters_matrices = NULL;
    int *words_and_grid_nb = NULL;
    int len_words_etc;

    letters_to_matrices(*letters,*grid_letters_index,*nb_grid_letters,*list_letters_index,*nb_list_letters,&grid_letters_matrices,&list_letters_matrices,&words_and_grid_nb,&len_words_etc);
    
    free_all(*letters,nb_letters,*grid_letters_index,*list_letters_index);
    */
    
    
    
    
    
}
