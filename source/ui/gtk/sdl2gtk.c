#include <gtk/gtk.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>


GtkWidget *GTKImageFromSDL_Surface(SDL_Surface *surface)
{
    Uint32 src_format;
    Uint32 dst_format;

    GdkPixbuf *pixbuf;
    gboolean has_alpha;
    int rowstride;
    guchar *pixels;

    GtkWidget *image;

    // select format
    src_format = surface->format->format;
    has_alpha = SDL_ISPIXELFORMAT_ALPHA(src_format);
    if (has_alpha)
    {
        dst_format = SDL_PIXELFORMAT_RGBA32;
    }
    else
    {
        dst_format = SDL_PIXELFORMAT_RGB24;
    }

    // create pixbuf
    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, has_alpha, 8,
                             surface->w, surface->h);
    rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    pixels = gdk_pixbuf_get_pixels(pixbuf);

    // copy pixels
    SDL_LockSurface(surface);
    SDL_ConvertPixels(surface->w, surface->h, src_format,
               surface->pixels, surface->pitch,
               dst_format, pixels, rowstride);
    SDL_UnlockSurface(surface);

    // create GtkImage from pixbuf
    image = gtk_image_new_from_pixbuf(pixbuf);

    // release our reference to the pixbuf
    g_object_unref(pixbuf);

    return image;
}

void GTKUpdateImageFromSDL_Surface(GtkWidget *image, SDL_Surface *surface)
{
    if (surface == NULL)
    {
        return;
    }

    if (image == NULL)
    {
        return;
    }

    Uint32 src_format;
    Uint32 dst_format;

    GdkPixbuf *pixbuf;
    gboolean has_alpha;
    int rowstride;
    guchar *pixels;

    // select format
    src_format = surface->format->format;
    has_alpha = SDL_ISPIXELFORMAT_ALPHA(src_format);
    if (has_alpha)
    {
        dst_format = SDL_PIXELFORMAT_RGBA32;
    }
    else
    {
        dst_format = SDL_PIXELFORMAT_RGB24;
    }

    // create pixbuf
    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, has_alpha, 8, surface->w, surface->h);
    rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    pixels = gdk_pixbuf_get_pixels(pixbuf);

    // copy pixels
    SDL_LockSurface(surface);
    SDL_ConvertPixels(surface->w, surface->h, src_format,
               surface->pixels, surface->pitch,
               dst_format, pixels, rowstride);
    SDL_UnlockSurface(surface);

    // set GtkImage from pixbuf
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);

    // release our reference to the pixbuf
    g_object_unref(pixbuf);
}
