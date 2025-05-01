#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

SDL_Surface* extractImage(SDL_Surface* surface, int x, int y, int w, int h)
{
    SDL_Surface* subSurface = SDL_CreateRGBSurface(0, w, h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
    SDL_Rect rect = {x, y, w, h};
    SDL_BlitSurface(surface, &rect, subSurface, NULL);
    return subSurface;
}

SDL_Surface* extractImageFromRect(SDL_Surface* surface, SDL_Rect rect)
{
    SDL_Surface* subSurface = SDL_CreateRGBSurface(0, rect.w, rect.h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
    SDL_BlitSurface(surface, &rect, subSurface, NULL);
    return subSurface;
}

#ifdef EXTRACT_IMAGE_TEST
int main()
{
    SDL_Surface* surface = IMG_Load("test.png");
    SDL_Surface* subSurface = extractImage(surface, 10, 10, 100, 100);

    SDL_SaveBMP(subSurface, "test.bmp");

    SDL_FreeSurface(surface);
    SDL_FreeSurface(subSurface);
    return 0;
}
#endif
