#include <gtk/gtk.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "source/image_treatment/image_to_bin/image_to_bin.h"
#include "source/image_treatment/detections/process.h"
#include "source/image_treatment/detections/Letter.h"
#include "source/neural_network/chars_reco/neural_network.h"
#include "source/image_treatment/display.h"
#include "source/ui/gtk/sdl2gtk.h"
#include "source/solver/solver.h"

#define APPLICATION_ID "org.wssolver"
#define APPLICATION_NAME "WSSolver"

#pragma region Globals

static GtkWidget* window = NULL;
static GtkWidget* statusLabel = NULL;
static GtkWidget* image = NULL;
static SDL_Surface* imageSurface = NULL;
static GtkWidget* tweaksLayout = NULL;
static int currentStep = 0;

Letter *letters = NULL;
int *grid_letters_index = NULL;
int nb_grid_letters = 0;
int *list_letters_index = NULL;
int nb_list_letters = 0;

char **wordlist = NULL;
char **grid = NULL;
int len_words = 0;

int **found = NULL;
int col;

#pragma endregion

#pragma region WidgetCreation

static GtkWidget* GTKUI_CreateButtonWContainer(const char* label, void (*callback)(void));
static GtkWidget* GTKUI_CreateButton(const char* label, void (*callback)(void));
static GtkWidget* GTKUI_CreateHorizontalLayout(GtkWidget** leftAligned, size_t leftAlignedLen, GtkWidget** middle, size_t middleLen, GtkWidget** rightAligned, size_t rightAlignedLen);
static GtkWidget* GTKUI_CreateVerticalLayout(GtkWidget** topAligned, size_t topAlignedLen, GtkWidget** bottomAligned, size_t bottomAlignedLen);
static GtkWidget* GTKUI_CreateOpenFileDialog(const char* title);
static GtkWidget* GTKUI_CreateSaveFileDialog(const char* title);

static GtkWidget* GTKUI_CreateButtonWContainer(const char* label, void (*callback)(void))
{
    GtkWidget *container;
    GtkWidget *button;

    container = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    button = GTKUI_CreateButton(label, callback);
    gtk_container_add(GTK_CONTAINER(container), button);

    return container;
}

static GtkWidget* GTKUI_CreateButton(const char* label, void (*callback)(void))
{
    GtkWidget *button = gtk_button_new_with_label(label);
    g_signal_connect(button, "clicked", G_CALLBACK(callback), NULL);
    return button;
}

static GtkWidget* GTKUI_CreateHorizontalLayout(GtkWidget** leftAligned, size_t leftAlignedLen, GtkWidget** middle, size_t middleLen, GtkWidget** rightAligned, size_t rightAlignedLen)
{
    GtkWidget* layout = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    for(size_t i = 0; i < leftAlignedLen; i++)
    {
        gtk_box_pack_start(GTK_BOX(layout), leftAligned[i], FALSE, FALSE, 0);
    }
    for(size_t i = 0; i < middleLen; i++)
    {
        // does not seem to allow proper center alignment
        gtk_box_pack_start(GTK_BOX(layout), middle[i], TRUE, TRUE, 0);
    }
    for(size_t i = 0; i < rightAlignedLen; i++)
    {
        gtk_box_pack_end(GTK_BOX(layout), rightAligned[i], FALSE, FALSE, 0);
    }

    return layout;
}

static GtkWidget* GTKUI_CreateVerticalLayout(GtkWidget** topAligned, size_t topAlignedLen, GtkWidget** bottomAligned, size_t bottomAlignedLen)
{
    GtkWidget* layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    for(size_t i = 0; i < topAlignedLen; i++)
    {
        gtk_box_pack_start(GTK_BOX(layout), topAligned[i], FALSE, FALSE, 0);
    }
    for(size_t i = 0; i < bottomAlignedLen; i++)
    {
        gtk_box_pack_end(GTK_BOX(layout), bottomAligned[i], TRUE, TRUE, 0);
    }

    return layout;
}

static GtkWidget* GTKUI_CreateOpenFileDialog(const char* title)
{
    GtkWidget* dialog = gtk_file_chooser_dialog_new(title, NULL, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
    return dialog;
}

static GtkWidget* GTKUI_CreateSaveFileDialog(const char* title)
{
    GtkWidget* dialog = gtk_file_chooser_dialog_new(title, NULL, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
    return dialog;
}

#pragma endregion

#pragma region HelperFunctions

static void UI_SetStatus(const char* status);

static void UI_SetImage(SDL_Surface* surface)
{
    GTKUpdateImageFromSDL_Surface(image, surface);
}

static void UI_SaveImage()
{
    GtkWidget* dialog = GTKUI_CreateSaveFileDialog("Save image as");
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if(result == GTK_RESPONSE_ACCEPT)
    {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if(access(filename, F_OK) == 0)
        {
            printf("File %s already exists! Overwriting\n", filename);
        }
        gdk_pixbuf_save(gtk_image_get_pixbuf(GTK_IMAGE(image)), filename, "png", NULL, NULL);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void UI_OpenImage()
{
    GtkWidget* dialog = GTKUI_CreateOpenFileDialog("Open image");
    while(1)
    {
        gint result = gtk_dialog_run(GTK_DIALOG(dialog));
        if(result == GTK_RESPONSE_ACCEPT)
        {
            char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            SDL_Surface* surface = IMG_Load_RW(SDL_RWFromFile(filename, "rb"), 1);
            if (!surface)
            {
                printf("Unable to load image %s! SDL_image Error: %s\n", filename, IMG_GetError());
                continue;
            }
            UI_SetImage(surface);
            imageSurface = surface;
            g_free(filename);
        }
        else if(result == GTK_RESPONSE_CANCEL) {}
        else
        {
            continue;
        }
        gtk_widget_destroy(dialog);
        break;
    }
    UI_SetStatus("Opened image");
    currentStep = 0;
}

static void UI_NextStep()
{
    switch(currentStep)
    {
        case 0:
            image_to_bin(imageSurface);
            UI_SetStatus("Binarisation");
            break;
        case 1:
            process(imageSurface, &letters, &grid_letters_index, &nb_grid_letters, &list_letters_index, &nb_list_letters);
            UI_SetStatus("Processing Image");
            break;
        case 2:
            big_brain(letters, grid_letters_index, nb_grid_letters, list_letters_index, nb_list_letters, &col, &len_words, &wordlist, &grid);
            UI_SetStatus("Neural Networking");
            break;
        case 3:
            process_solve(grid,wordlist,nb_grid_letters,col,len_words,&found);
            UI_SetStatus("Solving");
            break;
        case 4:
            display(imageSurface, letters, grid_letters_index, col, found);
            UI_SetStatus("Displaying");
            break;
        case 5:
            execvp("false", NULL);
            exit(0);
        default:
            UI_SetStatus("Finished");
            break;
    }
    
    UI_SetImage(imageSurface);
    currentStep++;
}

static void UI_SetStatus(const char* status)
{
    gtk_label_set_text(GTK_LABEL(statusLabel), status);
}

#pragma endregion

#pragma region Window Elements

static GtkWidget* GTKUI_LayoutTopBar()
{
    GtkWidget* saveCurrImage = GTKUI_CreateButtonWContainer("Save Current Image", &UI_SaveImage);
    statusLabel = gtk_label_new("\%STATUS\%");
    GtkWidget* nextStep = GTKUI_CreateButtonWContainer("Next Step", &UI_NextStep);
    // GtkWidget* fastforward = GTKUI_CreateButtonWContainer("Finish", &UI_Finish);
    // GtkWidget* rightAligned[2] = {nextStep, fastforward};

    // GtkWidget* layout = GTKUI_CreateHorizontalLayout(&saveCurrImage, 1, &statusLabel, 1, rightAligned, 2);

    GtkWidget* layout = GTKUI_CreateHorizontalLayout(&saveCurrImage, 1, &statusLabel, 1, &nextStep, 1);

    return layout;
}

static GtkWidget* GTKUI_LayoutRightPanel()
{
    // Right pannel contains all the tweaks we can make to the image at the current step

    GtkWidget* tweaksLabel = gtk_label_new("Tweaks");
    // Scrollable vertical region
    GtkWidget* scrollable = gtk_scrolled_window_new(NULL, NULL);
    /*GtkWidget* scrollableViewport = gtk_scrolled_window_get_child(GTK_SCROLLED_WINDOW(scrollable));
    tweaksLayout = GTKUI_CreateVerticalLayout(NULL, 0, NULL, 0);*/
    GtkWidget* layout = GTKUI_CreateVerticalLayout(&tweaksLabel, 1, &scrollable, 1);

    return layout;
}

static GtkWidget* GTKUI_LayoutImage()
{
    image = gtk_image_new();
    //GtkWidget* layout = GTKUI_CreateVerticalLayout(&image, 1, NULL, 0);
    return image;
}

static GtkWidget* GTKUI_LayoutWindow()
{
    GtkWidget* topBar = GTKUI_LayoutTopBar();
    GtkWidget* rightPanel = GTKUI_LayoutRightPanel();
    GtkWidget* image = GTKUI_LayoutImage();

    GtkWidget* mainlayout = GTKUI_CreateHorizontalLayout(NULL, 0, &image, 1, &rightPanel, 1);

    GtkWidget* window = GTKUI_CreateVerticalLayout(&topBar, 1, &mainlayout, 1);

    return window;
}

#pragma endregion

static void activate(GtkApplication* app, gpointer user_data)
{
    window = gtk_application_window_new(app);

    gtk_window_set_title(GTK_WINDOW(window), APPLICATION_NAME);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 450);
    gtk_widget_show_all(window);
    
    GtkWidget* layout = GTKUI_LayoutWindow();
    gtk_container_add(GTK_CONTAINER(window), layout);

    UI_OpenImage();

    gtk_widget_show_all(window);
    printf("Setting image\n");
}

int main(int argc, char **argv)
{
    GtkApplication *app;
    int status;


    app = gtk_application_new(APPLICATION_ID, G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    execvp("false", argv);

    return status;
}
