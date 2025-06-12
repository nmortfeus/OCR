#include <stdlib.h>
#include <err.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <time.h>
#include "image_to_bin.h"

double diff_timespec(const struct timespec *t1, const struct timespec *t0)
{
    double second = difftime(t1->tv_sec, t0->tv_sec);
    return second + ((double)t1->tv_nsec - (double)t0->tv_nsec) / 1e9;
}

void next_step(SDL_Surface* surface, unsigned char* image, unsigned long* integral,
                                                           unsigned long* sq_integral)
{
    static int step = 1;
    unsigned char locked = 0;

    if (SDL_MUSTLOCK(surface))
    {
        SDL_LockSurface(surface);
        locked = 1;
    }

    // Pre-processing
    switch (step)
    {
        case 1:
            gaussian_blur(surface);
            break;
        case 2:
            to_grayscale(surface, image, surface->w, surface->h);
            integral_image(surface->w, surface->h, image, integral,
                            sq_integral);
            change_surface(surface, image, surface->w, surface->h);
            break;
        case 3:
            apply_contrast(image, surface->w, surface->h, integral);
            change_surface(surface, image, surface->w, surface->h);
            break;
        case 4:
            sauvola(surface->w, surface->h, integral, sq_integral, image);
            change_surface(surface, image, surface->w, surface->h);
            break;
        case 5:
            cleanup_noise(image, surface->w, surface->h);
            change_surface(surface, image, surface->w, surface->h);
            break;
        default:
            step = 6;
            break;
    }

    if (locked)
        SDL_UnlockSurface(surface);

    step++;
}

void step_by_step(SDL_Surface* surface)
{
    SDL_Window* window = SDL_CreateWindow("image_to_bin", SDL_WINDOWPOS_CENTERED, 
                                                          SDL_WINDOWPOS_CENTERED, 
                                                          surface->w, surface->h, 
                                                          SDL_WINDOW_RESIZABLE);
    if (!window)
        errx(EXIT_FAILURE, "create_window failed: %s", SDL_GetError());

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer)
        errx(EXIT_FAILURE, "create_renderer failed: %s", SDL_GetError());

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture)
        errx(EXIT_FAILURE, "could not create texture: %s", SDL_GetError());

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    unsigned char* image = malloc(surface->w * surface->h * sizeof(unsigned char));
    if (!image)
        err(EXIT_FAILURE, "malloc()");
    unsigned long* integral = malloc(surface->w * surface->h * sizeof(unsigned long));
    if (!integral)
        err(EXIT_FAILURE, "malloc()");
    unsigned long* sq_integral = malloc(surface->w * surface->h * sizeof(unsigned long));
    if (!sq_integral)
        err(EXIT_FAILURE, "malloc()");

    int quit = 0;
    SDL_Event e;
    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN:
                    next_step(surface, image, integral, sq_integral);
                    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_RenderCopy(renderer, texture, NULL, NULL);
                    SDL_RenderPresent(renderer);
                    break;
            }
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    free(image);
    free(integral);
    free(sq_integral);
}

int main(int argc, char** argv)
{
    if (argc < 2 || argc > 3 || (argc==3 && strcmp(argv[2], "-p")))
        errx(EXIT_FAILURE, "Usage: image_to_bin <image path> <optional -p>");

    if (SDL_Init(SDL_INIT_VIDEO))
        errx(EXIT_FAILURE, "sdl_init failed: %s", SDL_GetError());

    int flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(flags) & flags))
        errx(EXIT_FAILURE, "img_init failed: %s", IMG_GetError());

    SDL_Surface* surface = IMG_Load(argv[1]);
    if (!surface)
        errx(EXIT_FAILURE, "surface failed: %s", IMG_GetError());
    

    struct timespec t0, t1;

    clock_gettime(CLOCK_MONOTONIC, &t0);

    // Functions go here
    
    if (argc == 2)
        image_to_bin(surface);
    else if (argc == 3)
    {
        step_by_step(surface);
        return EXIT_SUCCESS;
    }
    // ----

    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("time elapsed: %lfs\n", diff_timespec(&t1, &t0));

    SDL_Window* window = SDL_CreateWindow("image_to_bin", SDL_WINDOWPOS_CENTERED,
                                                          SDL_WINDOWPOS_CENTERED, 
                                                          surface->w, surface->h,
                                                          SDL_WINDOW_RESIZABLE);
    if (!window)
        errx(EXIT_FAILURE, "create_window failed: %s", SDL_GetError());

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer)
        errx(EXIT_FAILURE, "create_renderer failed: %s", SDL_GetError());

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture)
        errx(EXIT_FAILURE, "could not create texture: %s", SDL_GetError());

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    int quit = 0;
    SDL_Event e;
    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
                case SDL_QUIT:
                    quit = 1;
                    break;
            }
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}