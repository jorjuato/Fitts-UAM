//////////////////////////////////////////////////////////////////
//  draw.c
//
//
// Copyleft (L)		Marzo 2010		Jorge Ibáñez
//
//////////////////////////////////////////////////////////////////

//Cabecera principal
#ifndef __INCLUDESDL
#include "SDL/SDL.h"
//#include "SDL/SDL_opengl.h"
//#include "SDL/SDL_framerate.h"
#include "SDL/SDL_gfxPrimitives.h"
//#include "./font/nfontc.h"
//#include "SDL/SDL_gfxBlitFunc.h"
#define __INCLUDESDL
#endif

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

//------------------------------------------------------------  ShowBackground()
//
void ShowHandFeedback(float x,float y, Uint32 color){
	SDL_Rect fbRect;
	fbRect.x = SCN_ABS_SCALE_X(x-FEEDBACK_SIZE/2);
	fbRect.y = SCN_ABS_SCALE_Y(y+FEEDBACK_SIZE/2);
	fbRect.w = SCN_REL_SCALE_X(FEEDBACK_SIZE);
	fbRect.h = SCN_REL_SCALE_Y(FEEDBACK_SIZE);

	SDL_FillRect(screen, &fbRect, color);
	//putpixel(screen, SCN_ABS_SCALE_X(x), SCN_ABS_SCALE_Y(y), color);
}

//------------------------------------------------------------  ShowBackground()
//
void ShowHalfBackground(Uint32 color, char side){
	SDL_Rect bg;

	bg.w = WIDTH/2;
	bg.h = HEIGHT;
	bg.y = 0;

	if (side == 'R')
		bg.x = 0;
	else
		bg.x = WIDTH/2;

	SDL_FillRect(screen, &bg, color);
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
	//fflush(stdout);
}

//------------------------------------------------------------  ShowTarget()
//
void ShowTarget(Uint32 color, float ID, char side){
	SDL_Rect target;
	if (side == 'R') {
		target.x = (Sint16)SCN_ABS_SCALE_X(-TARGET_XMAX(ID));
		target.y = (Sint16)SCN_ABS_SCALE_Y(TARGET_YMAX);
		target.w = (Uint16)SCN_REL_SCALE_X(TARGET_WIDTH(ID));
		target.h = (Uint16)SCN_REL_SCALE_Y(TARGET_HEIGHT);
	}
	else {
		target.x = (Sint16)SCN_ABS_SCALE_X(TARGET_XMIN(ID));
		target.y = (Sint16)SCN_ABS_SCALE_Y(TARGET_YMAX);
		target.w = (Uint16)SCN_REL_SCALE_X(TARGET_WIDTH(ID));
		target.h = (Uint16)SCN_REL_SCALE_Y(TARGET_HEIGHT);
	}

	SDL_FillRect(screen, &target, color);

}


void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel){
	int bpp = surface->format->BytesPerPixel;
	// Here p is the address to the pixel we want to set
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp)	{
		case 1:
			*p = pixel;
			break;
		case 2:
			*(Uint16 *)p = pixel;
		break;
		case 3:
		if(SDL_BYTEORDER == SDL_BIG_ENDIAN)	{
			p[0] = (pixel >> 16) & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = pixel & 0xff;
		}
		else {
			p[0] = pixel & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = (pixel >> 16) & 0xff;
		}
		break;
		case 4:
			*(Uint32 *)p = pixel;
		break;
 	}
}
