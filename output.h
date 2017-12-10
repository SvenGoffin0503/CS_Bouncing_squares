#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"
#include "constants.h"

//Initializes the SDL output
void init_output(void);

//Draws a pixel on the surface at the x and y coordinates and coloring it using r,g,b values
void draw_pixel(SDL_Surface*, Uint32 x, Uint32 y, Uint8 r, Uint8 g, Uint8 b);

//Updates the SDL output given the content of the state table
void update_output(int table[SIZE_X][SIZE_Y]);

#endif //_OUTPUT_H_
