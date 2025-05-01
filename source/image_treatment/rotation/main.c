#include <err.h>
#include <math.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "rotate.h"

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        errx(EXIT_FAILURE, "Usage: %s <image path> <angle>", argv[0]);
    }

    //initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        errx(EXIT_FAILURE, "SDL_Init: %s\n", SDL_GetError());
    }

    //initialize SDL_image
    int flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(flags) & flags))
        errx(EXIT_FAILURE, "img_init failed: %s", IMG_GetError());

    //create window
    SDL_Window* window = SDL_CreateWindow("image_to_bin", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 500, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window)
        errx(EXIT_FAILURE, "create_window failed: %s", SDL_GetError());
    
    //create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer)
        errx(EXIT_FAILURE, "create_renderer failed: %s", SDL_GetError());

    //Load image
    SDL_Surface* surface = IMG_Load(argv[1]);
    if (!surface)
    {
        errx(EXIT_FAILURE, "IMG_Load: %s\n", IMG_GetError());
    }
    
    //Rotate image
    SDL_Surface* rotatedSurface = rotateman(surface, atof(argv[2]));
     if (!rotatedSurface)
      {
        SDL_FreeSurface(surface);
        SDL_Quit();
        return -1;
    }

    // Create a window and render the rotated image
    SDL_SetWindowSize(window, rotatedSurface->w, rotatedSurface->h);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, rotatedSurface);
    if (!texture)
    {
        errx(EXIT_FAILURE, "could not create texture: %s", SDL_GetError());
    }
    //save the image
    IMG_SavePNG(rotatedSurface, "rotated.png");

    // Display the rotated image
    SDL_RenderClear(renderer);
    SDL_Rect rect = {0, 0, rotatedSurface->w, rotatedSurface->h}; 
    SDL_RenderCopy(renderer, texture, NULL, &rect);
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



    //Free memory
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    
    return EXIT_SUCCESS;
    
}
