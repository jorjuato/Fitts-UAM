//////////////////////////////////////////////////////////////////
//  draw.c
//
//
// Copyleft (L)		Marzo 2010		Jorge Ibáñez
//
//////////////////////////////////////////////////////////////////

////Cabecera principal del sistema grafico SDL
//#ifndef __INCLUDESDL
//#include "SDL/SDL.h"
////#include "SDL/SDL_opengl.h"
////#include "SDL/SDL_framerate.h"
//#include "SDL/SDL_gfxPrimitives.h"
////#include "./font/nfontc.h"
////#include "SDL/SDL_gfxBlitFunc.h"
//#define __INCLUDESDL
//#endif

//Cabeceras con definiciones para el experimento
#include "display.h"
#include "fitts.h"
#include "draw.h"

// Puntero a la pantalla completa. Definida en sdlgl_control.c
extern SDL_Surface 	*screen;
//extern Uint32		g_init_time;


//------------------------------------------------------------  UpdateScreen()
//
void UpdateScreen(){
	SDL_Flip(screen);
}

//------------------------------------------------------------  ShowBackground()
//
void ShowBackground(Uint32 color){
	SDL_FillRect(screen, NULL, color);
}

//------------------------------------------------------------ ShowMesagge()
//
void ShowMessage(const char* msg){
	//SDL_Surface *text;
	//SDL_Color color = {255, 255, 255};

	//stringColor(screen, Sint16 x, Sint16 y, g_message,Uint32 color);
	//gfxPrimitivesSetFont(unsigned char *fontdata, int cw, int ch);

	/* Creates an SDL surface containing the specified text */
	//text = FNT_Render(msg, color);

	//SDL_BlitSurface(text, NULL, screen, NULL);
	//SDL_FreeSurface(text);

}

//------------------------------------------------------------  ShowTargets()
//
 void ShowTargets(Uint32 colorL, Uint32 colorR, float ID){
	SDL_Rect targetL, targetR;

	targetL.x = (Sint16)SCN_ABS_SCALE_X(-TARGET_XMAX(ID));
	targetL.y = (Sint16)SCN_ABS_SCALE_Y(TARGET_YMAX);
	targetL.w = (Uint16)SCN_REL_SCALE_X(TARGET_WIDTH(ID));
	targetL.h = (Uint16)SCN_REL_SCALE_Y(TARGET_HEIGHT);

	targetR.x = (Sint16)SCN_ABS_SCALE_X(TARGET_XMIN(ID));
	targetR.y = (Sint16)SCN_ABS_SCALE_Y(TARGET_YMAX);
	targetR.w = (Uint16)SCN_REL_SCALE_X(TARGET_WIDTH(ID));
	targetR.h = (Uint16)SCN_REL_SCALE_Y(TARGET_HEIGHT);

	SDL_FillRect(screen, &targetL, colorL);
	SDL_FillRect(screen, &targetR, colorR);

	//printf("ID = %f, Width = %f; SCNWidth = %d\n", ID, TARGET_WIDTH(ID), targetL.w);
	//printf("Test1 = %d; Test2 = %d\n", (Sint16)SCN_ABS_SCALE_X(MIN_X), (Sint16)SCN_ABS_SCALE_Y(MIN_Y));
	//printf("Lx = %d; Ly = %d Lw = %d; Lh = %d\n", targetL.x, targetL.y, targetL.w, targetL.h);
	//printf("Rx = %d; Ry = %d Rw = %d; Rh = %d\n", targetR.x, targetR.y, targetR.w, targetR.h);
	fflush(stdout);
}

////------------------------------------------------------------  LoadImage()
////
//SDL_Surface* LoadImage(char *file) {
	//SDL_Surface *tmp = SDL_LoadBMP(file);

	//if (!tmp)
		//fprintf(stderr, "Error: image file '%s' could not be opened: %s\n", file, SDL_GetError());
	//else
		//if(SDL_SetColorKey(tmp, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(tmp->format, COLORKEY)) == -1)
			//fprintf(stderr, "Warning: colorkey will not be used, reason: %s\n", SDL_GetError());

	//return SDL_DisplayFormat(tmp);
//}


