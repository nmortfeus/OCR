#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <math.h>

#define k 0.6
#define R 128
#define SAUVOLA_WINDOW 25
#define CONTRAST_WINDOW 35
#define CONTRAST_FACTOR 9.25

/// @brief apply gaussian blur with sigma = 3 to a surface
/// @param surface an sdl surface
void gaussian_blur(SDL_Surface* surface)
{
    Uint32* pixels = surface->pixels;
    double kernel[3][3] = { {1/16.0, 1/8.0, 1/16.0},
                            {1/8.0, 1/4.0, 1/8.0},
                            {1/16.0, 1/8.0, 1/16.0} };

    // Copy old pixel values
    Uint32* old_values = malloc(surface->w * surface->h * sizeof(Uint32));
    if (!old_values)
        err(EXIT_FAILURE, "malloc()");

    memcpy(old_values, pixels, surface->w * surface->h * sizeof(Uint32));


    for (int x = 1; x < surface->w - 1; x++)
    {
        for (int y = 1; y < surface->h - 1; y++)
        {
            double sum_r = 0, sum_g = 0, sum_b = 0;
            for (int i = -1; i <= 1; i++)
            {
                for (int j = -1; j <= 1; j++)
                {
                    Uint32 pixel = old_values[(x+j) + (y+i)*surface->w];
                    Uint8 r, g, b;

                    SDL_GetRGB(pixel, surface->format, &r, &g, &b);

                    sum_r += (double)r*kernel[j+1][i+1];
                    sum_g += (double)g*kernel[j+1][i+1];
                    sum_b += (double)b*kernel[j+1][i+1];
                }
            }
            
            pixels[x + y*surface->w] = SDL_MapRGB(surface->format, (Uint8)sum_r
                                                , (Uint8)sum_g, (Uint8)sum_b);
        }
    }

    free(old_values);
}

//! !! NOT IN USE FOR NOW !!
/// @brief adjust the dynamic range of an image
/// @param image an array of values from 0 to 255 representing a grayscale image
/// @param w width of image
/// @param h height of image
/// @param min minimum value of the image
/// @param max maximum value of the image
void adjust_dynamic_range(unsigned char* image, int w, int h, unsigned char min, unsigned char max)
{
    for (int x = 0; x < w; x++)
    {
        for (int y = 0; y < h; y++)
        {
            image[x + y*w] = (image[x + y*w] - min) * 255/(max-min);
        }
    }
}

/// @brief convert and sdl surface to a grayscale image and store the result into image
/// @param surface an sdl surface
/// @param image the array to store the result into
/// @param w width of the image
/// @param h height of the image
void to_grayscale(SDL_Surface* surface, unsigned char* image, int w, int h)
{
    Uint32* pixel = surface->pixels;

    // UNCOMMENT TO ADJUST DYNAMIC RANGE
    // unsigned char min = 255;
    // unsigned char max = 0;

    for (int x = 0; x < w; x++)
    {
        for (int y = 0; y < h; y++)
        {
            Uint32* p = &pixel[y * surface->w + x];
            Uint8 r, g, b;
            SDL_GetRGB(*p, surface->format, &r, &g, &b);

            unsigned char grey = 0.2126*r + 0.7152*g + 0.0722*b;
            image[x + y*w] = grey;

            // UNCOMMENT TO ADJUST DYNAMIC RANGE
            // if (grey > max)
            //     max = grey;
            // if (grey < min)
            //     min = grey;
        }
    }
    // UNCOMMENT TO ADJUST DYNAMIC RANGE
    // adjust_dynamic_range(image, w, h, min, max);
}

/// @brief calculate the integral and the square integral images of a grayscale image
/// @param w width of the image
/// @param h height of the image
/// @param src the source image
/// @param dest array to store the integral image into
/// @param sq_dest array to store the square integral image into
void integral_image(int w, int h, unsigned char* src, unsigned long* dest, unsigned long* sq_dest)
{
    // Initialize first value
    dest[0] = src[0];
    sq_dest[0] = src[0] * src[0];

    // Generate first line of the integral images
    for (int x = 1; x < w; x++)
    {
        dest[x] = dest[x-1] + src[x];
        sq_dest[x] = sq_dest[x-1] + (src[x] * src[x]);
    }

    // Generate first row of the integral images
    for (int y = 1; y < h; y++)
    {
        dest[0 + y*w] = dest[0 + (y-1)*w] + src[0 + y*w];
        sq_dest[0 + y*w] = sq_dest[0 + (y-1)*w] + (src[0 + y*w] * src[0 + y*w]);
    }

    // Generate rest of the integral images
    for (int x = 1; x < w; x++)
    {
        for (int y = 1; y < h; y++)
        {
            dest[x + y*w] = src[x + y*w] 
                            + dest[x-1 + y*w] 
                            + dest[x + (y-1)*w] 
                            - dest[x-1 + (y-1)*w];

            sq_dest[x + y*w] = (src[x + y*w] * src[x + y*w]) 
                                + sq_dest[x-1 + y*w] 
                                + sq_dest[x + (y-1)*w] 
                                - sq_dest[x-1 + (y-1)*w];
        }
    }
}

/// @brief Calculates the sum of the pixel values of an area using an integral image
/// @param w width of integral image
/// @param integral the integral image
/// @param x1 x coordinate of the top left corner
/// @param y1 y coordinate of the top left corner
/// @param x2 x coordinate of the bottom right corner
/// @param y2 y coordinate of the bottom right corner
/// @return sum of the pixel values of the area
unsigned int get_window_value(int w, unsigned long* integral, int x1, int y1, int x2, int y2)
{
    unsigned int A = (x1 > 0 && y1 > 0) ? integral[x1-1 + (y1-1)*w] : 0;
    unsigned int B = (y1 > 0) ? integral[x2 + (y1-1)*w] : 0;
    unsigned int C = (x1 > 0) ? integral[x1-1 + y2*w] : 0;
    unsigned int D = integral[x2 + y2*w];

    return D + A - B - C;
}

/// @brief Apply a contrast to an image using a local mean
/// @param image array representing the image
/// @param w width of the image
/// @param h height of the image
/// @param integral integral image used to calculate the local mean
void apply_contrast(unsigned char* image, int w, int h, unsigned long* integral)
{
    // UNCOMMENT TO ADJUST DYNAMIC RANGE
    // unsigned char min = 255;
    // unsigned char max = 0;
    int half = CONTRAST_WINDOW/2;
    double factor = CONTRAST_FACTOR;

    for (int x = 0; x < w; x++)
    {
        for (int y = 0; y < h; y++)
        {
            int x1 = x - half > 0 ? x - half : 0;
            int y1 = y - half > 0 ? y - half : 0;
            int x2 = x + half <= w-1 ? x + half : w-1;
            int y2 = y + half <= h-1 ? y + half : h-1;

            int pixel_nb = (x2 - x1 + 1) * (y2 - y1 + 1);

            double mean = (double)get_window_value(w, integral, x1, y1, x2, y2) / pixel_nb;
            long value = (image[x + y*w] - mean) * factor + mean;

            if (value > 255)
                value = 255;
            if (value < 0)
                value = 0;

            // UNCOMMENT TO ADJUST DYNAMIC RANGE
            // if (value > max)
            //     max = value;
            // if (value < min)
            //     min = value;

            image[x + y*w] = value;
        }
    }
    // UNCOMMENT TO ADJUST DYNAMIC RANGE
    // adjust_dynamic_range(image, w, h, min, max);
}

/// @brief Apply Sauvola thresholding algorithm to an image using integral images
/// @param w width of the image
/// @param h height of the image
/// @param integral integral image
/// @param sq_integral square integral image
/// @param image image
void sauvola(int w, int h, unsigned long* integral, unsigned long* sq_integral, unsigned char* image)
{
    int half = SAUVOLA_WINDOW/2;

    for (int x = 0; x < w; x++)
    for (int y = 0; y < h; y++)
    {
        int x1 = x - half > 0 ? x - half : 0;
        int y1 = y - half > 0 ? y - half : 0;
        int x2 = x + half <= w-1 ? x + half : w-1;
        int y2 = y + half <= h-1 ? y + half : h-1;

        int pixel_nb = (x2 - x1 + 1) * (y2 - y1 + 1);

        unsigned long sum = get_window_value(w, integral, x1, y1, x2, y2);
        unsigned long sum_sq = get_window_value(w, sq_integral, x1, y1, x2, y2);

        long double mean = (long double)sum / pixel_nb;
        long double var = ((long double)sum_sq / pixel_nb) - (mean * mean);
        long double std_d = var > 0 ? sqrtl(var) : 0;

        unsigned char theshold = mean * (1. + k * ((std_d/R) - 1.));
        image[x + y * w] = image[x + y * w] >= theshold ? 255 : 0;
    }
}

/// @brief Count the number of pixels in a block of black pixels
/// @param image array of pixels
/// @param w width of the image
/// @param h height of the image
/// @param x x coordinate of the current pixel
/// @param y y coordinate of the current pixel
/// @param visited array keeping track of already visited pixels
/// @return number of black pixels in the block
int count_block_size(unsigned char* image, int w, int h, int x, int y, unsigned char* visited)
{
    // If pixel should not be counted
    if (visited[x + y*w] || image[x + y*w] == 255)
        return 0;

    visited[x + y*w] = 1;
    int c = 1;

    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            // Skip out of bounds pixels
            if (x+i < 0 || x+i >= w || y+j < 0 || y+j >=h)
                continue;

            c += count_block_size(image, w, h, x+i, y+j, visited);
        }
    }

    return c;
}

/// @brief set a block of black pixels to white
/// @param image array of pixels
/// @param w width of the image
/// @param h height of the image
/// @param x x coordinate of the current pixel
/// @param y y coordinate of the current pixel
void erase_block(unsigned char* image, int w, int h, int x, int y)
{
    // Erase pixel
    image[x + y*w] = 255;

    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            // If pixel is out of bounds or white
            if (x+i < 0 || x+i >= w || y+j < 0 || y+j >=h 
                        || image[x+i + (y+j)*w] == 255)
                        continue;

            erase_block(image, w, h, x+i, y+j);
        }
    }
}

/// @brief search for all the blocks of black pixels and turn them white
/// @param image array of pixels
/// @param w width of the image
/// @param h height of the image
void cleanup_noise(unsigned char* image, int w, int h)
{
    unsigned char* visited = calloc(w*h, sizeof(unsigned char));
    if (!visited)
        err(EXIT_FAILURE, "calloc()");

    for (int x = 0; x < w; x++)
    {
        for (int y = 0; y < h; y++)
        {
            // Skip visited and white pixels 
            if (visited[x + y*w] || image[x + y*w] == 255)
                continue;

            int count = count_block_size(image, w, h, x, y, visited);

            // Delete small blocks
            if (count < 40)
                erase_block(image, w, h, x, y);
        }
    }

    free(visited);
}

/// @brief update surface with the given array containing the values for each pixel
/// @param surface an sdl surface
/// @param image an array of values corresponding to the pixels of surface
void change_surface(SDL_Surface* surface, unsigned char* image, int w, int h)
{
    Uint32* pixel = surface->pixels;

    for (int x = 0; x < w; x++)
    {
        for (int y = 0; y < h; y++)
        {
            Uint8 value = image[x + y * w];
            pixel[y * surface->w + x] = SDL_MapRGB(surface->format, value, value, value);
        }
    }
}

/// @brief binarize an image
/// @param surface sdl surface containing the image to binarize
void image_to_bin(SDL_Surface* surface)
{
    unsigned char locked = 0;

    unsigned char* image = malloc(surface->w * surface->h * sizeof(unsigned char));
    if (!image)
        err(EXIT_FAILURE, "malloc()");
    unsigned long* integral = malloc(surface->w * surface->h * sizeof(unsigned long));
    if (!integral)
        err(EXIT_FAILURE, "malloc()");
    unsigned long* sq_integral = malloc(surface->w * surface->h * sizeof(unsigned long));
    if (!sq_integral)
        err(EXIT_FAILURE, "malloc()");

    if (SDL_MUSTLOCK(surface))
    {
        SDL_LockSurface(surface);
        locked = 1;
    }

    // Pre-processing
    gaussian_blur(surface);
    to_grayscale(surface, image, surface->w, surface->h);
    integral_image(surface->w, surface->h, image, integral, sq_integral);
    apply_contrast(image, surface->w, surface->h, integral);

    // Processing
    sauvola(surface->w, surface->h, integral, sq_integral, image);

    // Postprocessing
    cleanup_noise(image, surface->w, surface->h);

    // Update the surface
    change_surface(surface, image, surface->w, surface->h);

    if (locked)
        SDL_UnlockSurface(surface);

    free(image);
    free(integral);
    free(sq_integral);
}