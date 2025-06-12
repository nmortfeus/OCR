#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include "../rotation/rotate.h"

const double pi = 3.1415926;

// 800 for threshold
double to_pixel_color(double pixel) {
    if (pixel < 800)
        return 0;
    else
        return 255;
}

// Accumulate lines
void accumulate(int *accumulator, int x, int y, int size){
    for (double theta = 0; theta < 180; theta++){
        double rho = x * cos(theta * pi / 180.0) + y * sin(theta * pi / 180.0);
        if (rho >= 0 && (int)rho < size){
            accumulator[(int)(size * theta + rho)] += 1;
        }
    }

}

int *hough_transform(SDL_Surface *surface, int *to_return_size) {
    SDL_LockSurface(surface);

    size_t img_w = surface->w;
    size_t img_h = surface->h;

    // Detection of edges using canny algorithm
    float gx[3][3] = {{1, 0, -1}, {2, 0, -2}, {1, 0, -1}};
    float gy[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};

    Uint32 *surfacePixels = surface->pixels;
    if (!surfacePixels) {
        errx(EXIT_FAILURE, "%s", SDL_GetError());
    }
    int pitch = surface->pitch / sizeof(Uint32);
    int size = sqrt(img_w*img_w+img_h*img_h);
    int *accumulator = calloc(180*size,sizeof(int));
    for (size_t i = 1; i < img_h - 1; i++) {
        for (size_t j = 1; j < img_w - 1; j++) {
            float xpart = 0;
            float ypart = 0;

            for (int k = -1; k < 2; k++) {
                for (int l = -1; l < 2; l++) {
                    Uint8 p = surfacePixels[(i+k) * pitch + j + l];
                    xpart += p * gx[1 + l][1 + k];
                    ypart += p * gy[1 + l][1 + k];
                }
            }
            //Hough transform for edge detected
            if (to_pixel_color(sqrt(xpart*xpart + ypart*ypart))==255){
                accumulate(accumulator,i,j,size);
            }
        }
    }
    *to_return_size = size;
    return accumulator;

    SDL_UnlockSurface(surface);
}

void print_accumulator_in_array(int size, int *array, int len){
    printf("[ ");
    for (int i = 0; i < len; i++){
        printf("(rho:%i,theta:%i) ", array[i]%size,array[i]/size);
    }
    printf("]\n");
}

// Return true if it can be a line of the grid of a well oriented image
int good_line(int index, int size){
    return index/size == 90 || index/size == 0;
}

// Return true if there is a line duplicate in the array
int check_for_duplicate(int *array, int size, int nb, int index){
    for (int i = 0; i < nb; i++){
        if (abs(index%size-array[i]%size)<2 && abs(index/size-array[i]/size)<2){
            return i+1;
        }
    }
    return 0;
}

// Return most obvious lines (maximum is max) in the accumulator (from most obvious to less obvious)
int *maxInAccumulator(int *accumulator, size_t len, int threshold, int *finalnb, size_t max){
    int *to_return = NULL;
    size_t nb_lines = 0;
    for (size_t i = 0; i < len; i++){
        // Add " || !good_line(i,len/180)" when we assume it is a well oriented image
        if ((nb_lines==max && accumulator[i]<accumulator[to_return[nb_lines-1]]) || accumulator[i]<threshold || !good_line(i,len/180)){
            continue;
        }
        int duplicate = check_for_duplicate(to_return,len/180,nb_lines,i);
        if (duplicate){
            // More obvious dupplicate line
            if (accumulator[i]>accumulator[to_return[duplicate-1]]){
                to_return[duplicate-1] = 0;
            }
            else{
                continue;
            }
        }
        if (nb_lines<max && !duplicate){
            to_return = realloc(to_return, (nb_lines+1)*sizeof(int));
            to_return[nb_lines] = 0;
            nb_lines++;
        }
        size_t j = 0;
        while (j < nb_lines && accumulator[i] < accumulator[to_return[j]]){
            j++;
        }
        int temp = i;
        while (j < nb_lines && temp){
            int temp2  = to_return[j];
            to_return[j] = temp;
            temp = temp2;
            j++;
        }
    }
    *finalnb = nb_lines;
    return to_return;
}

// Show the line with rho and theta for test
void show_line(SDL_Surface *surface, int rho, int theta) {
    Uint32 red = SDL_MapRGB(surface->format, 255, 0, 0);
    int pitch = surface->pitch / sizeof(Uint32);
    double right = cos(theta*pi/180.0);
    double left = sin(theta*pi/180.0);
    // Horizontal line
    if (theta == 0){
        for (int i = 0; i < surface->w; i++){
            ((Uint32*)surface->pixels)[rho * pitch + i] = red;
        }
    }
    // Vertical line
    else if(theta==90){
        for (int y = 0; y < surface->h; y++){
            ((Uint32*)surface->pixels)[y * pitch + (int)rho] = red;
        }
    }
    // Diagonal line more vertical
    else if (fabs(left) > fabs(right)){
        for (int y = 0; y < surface->h; y++){
            double x = (rho-y*right)/left;
            if (x >= 0 && x < surface->w){
                ((Uint32*)surface->pixels)[y * pitch + (int)x] = red;
            }
        }
    }
    else{
        for (int x = 0; x < surface->w; x++){
            double y = (rho-x*left)/right;
            if (y >= 0 && y < surface->h){
                ((Uint32*)surface->pixels)[(int)y * pitch + x] = red;
            }
        }
    }
}

// Try to detect the grid with the most obvious lines
void detect_grid(SDL_Surface *surface,int *maxIndex, int nb, int size, int *x1, int*x2, int *y1, int *y2){
    int leftmost = size;
    int rightmost = 0;
    int topmost = size;
    int bottommost = 0;
    for (int i = 0; i < nb; i++){
        int theta = maxIndex[i]/size;
        if(theta <= 1 && maxIndex[i]%size>2 && maxIndex[i]%size<size-1){
            if (maxIndex[i]%size > bottommost){
                bottommost = maxIndex[i]%size;
            }
            if(maxIndex[i]%size < topmost){
                topmost = maxIndex[i]%size;
            }
        }
        else if (abs(theta-90)<=1 && maxIndex[i]%size>2 && maxIndex[i]%size<size-1){
            if (maxIndex[i]%size > rightmost){
                rightmost = maxIndex[i]%size;
            }
            if (maxIndex[i]%size < leftmost){
                leftmost = maxIndex[i]%size;
            }
        }
    }
    if (leftmost == size || rightmost == 0 || topmost == size || bottommost == 0 || (double)(bottommost-topmost)/(rightmost-leftmost)<0.8 || (double)(bottommost-topmost)/(rightmost-leftmost)>1.2 ||
    bottommost-topmost < 400 || rightmost-leftmost < 400){
        *x1 = 0;
        *y1 = 0;
        *x2 = surface->w;
        *y2 = surface->h;
    }
    else{
        /*
        show_line(surface,topmost,0);
        show_line(surface,bottommost,0);
        show_line(surface,rightmost,90);
        show_line(surface,leftmost,90);
        */
        *x1 = leftmost;
        *y1 = topmost;
        *x2 = rightmost;
        *y2 = bottommost;
    }
}

void process_hough(SDL_Surface *surface, int *x1, int*x2, int *y1, int *y2){
    int size;
    int *accumulator = hough_transform(surface,&size);
    int maxIndexAngle = -1;
    int maxAux = 0;
    for(int i = 0; i < size*180; i++){
        if (maxIndexAngle == -1 || accumulator[i]>maxAux){
            maxIndexAngle = i;
            maxAux = accumulator[i];
        }
    }
    if ((maxIndexAngle/size)%90 != 0){
        *surface = *rotateman(surface,((maxIndexAngle/size)%90)-90);
        accumulator = hough_transform(surface,&size);
    }
    
    int threshold = 200;
    size_t maximum = 30;

    int nb;
    int *maxIndex = maxInAccumulator(accumulator, size*180,threshold,&nb, maximum);

    /*
    // For test
    for(int i = 0; i < nb; i++){
        if (!accumulator[maxIndex[i]]){
            continue;
        }
        int maxRho = maxIndex[i]%size;
        int maxTheta = maxIndex[i]/size;
        printf("%i-eme (%i) : Max Rho= %i and max Theta = %i\n",i, accumulator[maxIndex[i]], maxRho, maxTheta);
    }
    */

    // Detection of grid ?
    detect_grid(surface,maxIndex,nb,size,x1,x2,y1,y2);

    free(maxIndex);
    free(accumulator);
}