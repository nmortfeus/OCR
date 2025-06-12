#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include "process.h"
#include "../image_to_bin/image_to_bin.h"
#include "Letter.h"

int main(int argc, char **argv) {
    if (argc != 2)
        errx(EXIT_FAILURE, "Usage: <image>");

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    SDL_Window *window = SDL_CreateWindow("Surface detection", 0, 0, 0, 0, SDL_WINDOW_SHOWN);
    if (window == NULL)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    SDL_Surface *surface = IMG_Load(argv[1]);
    if (surface == NULL)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    SDL_SetWindowSize(window, surface->w, surface->h);

    image_to_bin(surface);

    Letter *letters = NULL;
    int *grid_letters_index = NULL;
    int nb_grid_letters = 0;
    int *list_letters_index = NULL;
    int nb_list_letters = 0;
    process(surface, &letters, &grid_letters_index, &nb_grid_letters, &list_letters_index, &nb_list_letters);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    SDL_Event event;
    while (1) {
        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT) {
            SDL_DestroyTexture(texture);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return EXIT_SUCCESS;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            SDL_DestroyTexture(texture);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return EXIT_SUCCESS;
        }
    }
    
    return EXIT_SUCCESS;
}