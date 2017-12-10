#include "output.h"

//Represents the screen
SDL_Surface* screen;
//Possible colors
Uint8 colors [5][3];

void init_output(){
 
  	//Hard-coding the colour values
	  colors[0][0] = 0x000000; colors[0][1] = 0x000000; colors[0][2] = 0x000000; //white
  	colors[1][0] = 0x0000FF; colors[1][1] = 0x0000FF; colors[1][2] = 0x0000FF; //
  	colors[2][0] = 0x0000FF; colors[2][1] = 0x000000; colors[2][2] = 0x000000; //red
  	colors[3][0] = 0x000000; colors[3][1] = 0x0000FF; colors[3][2] = 0x000000; //green
  	colors[4][0] = 0x000000; colors[4][1] = 0x000000; colors[4][2] = 0x0000FF; //blue
  
  	//SDL init
	  if(SDL_Init(SDL_INIT_VIDEO < 0)){
    	  printf("ERROR : Initialisation SDL\n");
    	  exit(-1);
  	}

  	//Prepare for exit
  	atexit(SDL_Quit);
  
  	//Setting video mode
  	screen = SDL_SetVideoMode(SIZE_Y * PIXEL_WIDTH, SIZE_X * PIXEL_WIDTH,
							32, SDL_SWSURFACE);
	
  	if(screen == NULL){
    	  printf("ERROR : Video Mode");
    	  exit(EXIT_FAILURE);
  	}

	  //Setting title
  	SDL_WM_SetCaption("Square bounce","");
}

void update_output(int table[SIZE_X][SIZE_Y]){
	  int i, j, x, y, red, green, blue;
		
	  //For each pixel in the table
	  for(i = 0; i < SIZE_X; ++i){
		  for(j = 0; j < SIZE_Y; ++j){
	  
			  //Get RBG value for that pixel state
			  red = colors[table[i][j]][0];
			  green = colors[table[i][j]][1];
			  blue = colors[table[i][j]][2];
    
	 		  //Draw the corresponding color on the screen
			  for (x = 0; x < PIXEL_WIDTH; x++) {
				  for (y = 0; y < PIXEL_WIDTH; y++) {
					  draw_pixel(screen, j*PIXEL_WIDTH+y, i*PIXEL_WIDTH+x,
							  	red, green, blue);
				  }
			  }
		  }
	  }

	//Makes the update happen on screen
	SDL_UpdateRect(screen, 0, 0, SIZE_Y * PIXEL_WIDTH,
					SIZE_X * PIXEL_WIDTH);
}

void draw_pixel(SDL_Surface* screen,
		Uint32 x,
		Uint32 y,
		Uint8 r,
		Uint8 g,
		Uint8 b){
	
    //Get the colour as and int given r,g,b values and the screen format
  	Uint32 color = SDL_MapRGB(screen->format, r, g, b);
  
  	//Get a pointer to the wanted pixel and change its colour
  	Uint32* bufp;
  	bufp = (Uint32*) screen->pixels + y*screen->pitch/4 + x;
  	*bufp = color;
}
