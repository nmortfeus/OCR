#include "rotate.h"


Uint32 bilinearInterpolation(SDL_Surface* surface, double x, double y)
{
    int x1 = (int)  x;
    int x2 = x1 + 1;
    int y1 = (int) y;
    int y2 = y1 + 1;

    // Ensure that the four points are within the image
    x1 = SDL_clamp(x1, 0, surface->w - 1);
    x2 = SDL_clamp(x2, 0, surface->w - 1);
    y1 = SDL_clamp(y1, 0, surface->h - 1);
    y2 = SDL_clamp(y2, 0, surface->h - 1);

    // Get pixel values
    Uint32* pixels = (Uint32*)surface->pixels;
    Uint32 topLeft = pixels[y1 * surface->w + x1];
    Uint32 topRight = pixels[y1 * surface->w + x2];
    Uint32 bottomLeft = pixels[y2 * surface->w + x1];
    Uint32 bottomRight = pixels[y2 * surface->w + x2];

    // Calculate weights
    double dx = x - x1;
    double dy = y - y1;

    Uint8 r,g,b,a;
    Uint8 r1,g1,b1,a1,r2,g2,b2,a2;

    //interpolate the pixel in x direction
    SDL_GetRGBA(topLeft, surface->format, &r1, &g1, &b1, &a1);
    SDL_GetRGBA(topRight, surface->format, &r2, &g2, &b2, &a2);
    Uint8 R1r = r1 * (1 - dx) + r2 * dx;
    Uint8 R1g = g1 * (1 - dx) + g2 * dx;
    Uint8 R1b = b1 * (1 - dx) + b2 * dx;
    Uint8 R1a = a1 * (1 - dx) + a2 * dx;

    SDL_GetRGBA(bottomLeft, surface->format, &r1, &g1, &b1, &a1);
    SDL_GetRGBA(bottomRight, surface->format, &r2, &g2, &b2, &a2);
    Uint8 R2r = r1 * (1 - dx) + r2 * dx;
    Uint8 R2g = g1 * (1 - dx) + g2 * dx;
    Uint8 R2b = b1 * (1 - dx) + b2 * dx;
    Uint8 R2a = a1 * (1 - dx) + a2 * dx;

    //interpolate the pixel in y direction
    r = R1r * (1 - dy) + R2r * dy;
    g = R1g * (1 - dy) + R2g * dy;
    b = R1b * (1 - dy) + R2b * dy;
    a = R1a * (1 - dy) + R2a * dy;

    return SDL_MapRGBA(surface->format, r, g, b, a);



    
}

void calculate_new_dimensions(int w, int h, double angle, int* newW, int* newH)
{
    double rad = angle * M_PI / 180;
    double s = sin(rad);
    double c = cos(rad);

    // Calculate new dimensions
    *newW = (int) (fabs(w * c) + fabs(h * s));
    *newH = (int) (fabs(w * s) + fabs(h * c));

     // Debug logs
    SDL_Log("Original Width: %d, Original Height: %d", w, h);
    SDL_Log("New Width: %d, New Height: %d", *newW, *newH);
}

SDL_Surface* rotateman(SDL_Surface* surface, double angleinput)
{
    const int width = surface-> w;
    const int height = surface->h;

    const double cX = ((double)width / 2.0);
    const double cY = ((double)height / 2.0);

    //Calculate new dimensions
    int newW, newH;
    calculate_new_dimensions(width, height, angleinput, &newW, &newH);

  
    // Create output surface
    SDL_Surface* rotated = SDL_CreateRGBSurfaceWithFormat(0,newW,newH,32,surface -> format -> format);
    if (!rotated)
    {
        SDL_Log("Failed to create surface: %s", SDL_GetError());
        return NULL;
    }

    //Calculate center of source and destination images
    double newcX = ((double)newW / 2.0);
    double newcY = ((double)newH / 2.0);


    //Precompute sin/cos
    double angle = angleinput * M_PI / 180;
    double cosA = cos(angle);
    double sinA = sin(angle);


    // Iterate over each pixel in the destination image
    for (int y = 0; y < newH; y++)
    {
        for (int x = 0; x < newW; x++)
        {
            // Map destination pixel to source pixel
            double srcX = cosA * (x - newcX) + sinA * (y - newcY) + cX;
            double srcY = - sinA * (x - newcX) + cosA * (y - newcY) + cY;


          // Check bounds and apply bilinear interpolation if within bounds
            if (srcX >= 0 && srcX < width && srcY >= 0 && srcY < height) 
            {
                Uint32 pixelColor = bilinearInterpolation(surface, srcX, srcY);
                ((Uint32*)rotated->pixels)[y * newW + x] = pixelColor;
            }
             else 
            {
                // Set out-of-bound pixels to transparent or black
                ((Uint32*)rotated->pixels)[y * newW + x] = SDL_MapRGBA(rotated->format, 255, 255, 255, 255);
            }
    

        }
        
    }

    return rotated;
            
        
}
