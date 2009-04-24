#include <stdio.h>

#include "bbc.h"
#include "sprite.h"

extern char DisplayFormat;
extern char PaletteOption;

static struct
{
    int red, green, blue;
} colour_error;

void display_init(int width, int height)
{

}

void display_finished(void)
{
}

void display_close(void)
{
}

void display_plot(int x, int y, unsigned char Red, unsigned char Green, unsigned char Blue)
{
    printf("*%d,%d,%d,%d,%d\n", x, y, Red, Green, Blue);
}
