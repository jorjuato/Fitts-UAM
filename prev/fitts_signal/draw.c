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

//Cabeceras con definiciones para creacion de hilos y señales
#include <pthread.h>
#include <signal.h>

//Cabeceras con definiciones para el experimento
#include "display.h"
#include "fitts.h"
#include "draw.h"

//EXTERNAL GLOBAL STATE VARIABLES
extern SDL_Surface 	*screen;				// Puntero a la pantalla completa. Definida en sdlgl_control.c
extern Uint32		g_init_time;
extern float g_ID;
extern char g_side;
extern Uint8 g_trial_state;

//MULTITHREADING GLOBALS
pthread_mutex_t g_sig_mutex;
pthread_t g_GraphThread_id;


//------------------------------------------------------------  InitGraphicsThread()
//
void InitGraphicsThread(){
	sigset_t signal_set;
	pthread_attr_t tattr;

	pthread_attr_init(&tattr);
	pthread_attr_setdetachstate(&tattr,PTHREAD_CREATE_DETACHED);

	//sigemptyset( &signal_set );
	sigfillset( &signal_set );
	//sigdelset(&signal_set, SIGUSR1);
	//pthread_sigmask(SIG_BLOCK, &signal_set, NULL );
	pthread_sigmask(SIG_BLOCK, &signal_set, NULL );
	if ( pthread_create(&g_GraphThread_id,NULL,GraphicsThread,NULL) ) {
		perror("Error in graphics thread, closing program...\n");
		exit(0);
	}
}

//------------------------------------------------------------  EndGraphicsThread()
//
void EndGraphicsThread(){
	ExperimentGraphics();
	if (pthread_kill(g_GraphThread_id, SIGUSR2))
		EndGraphicsThread();
}

//------------------------------------------------------------  ExperimentGraphics()
//
void ExperimentGraphics(){
	if (pthread_kill(g_GraphThread_id, SIGUSR1))
		ExperimentGraphics();
}

//------------------------------------------------------------  GraphicsThread()
//
void* GraphicsThread() {
	sigset_t signal_set;
	int sig;
	float ID;
	char side;
	Uint8 state;

	while(1) {
		//wait for SIGUSR1 signals
		sigemptyset( &signal_set );
		sigaddset(&signal_set, SIGUSR1);
		sigaddset(&signal_set, SIGUSR2);
		sigwait( &signal_set, &sig );
		//printf("Received signal %d in graphics thread\n", sig);
		//fflush(stdout);
		if (sig == SIGUSR1) {
			//Lock global variables
			pthread_mutex_lock(&g_sig_mutex);
			//Copy to local ones
			ID = g_ID;
			side = g_side;
			state = g_trial_state;
			//Unlock again
			pthread_mutex_unlock(&g_sig_mutex);
			//Do real plotting
			GraphicsFSA(state, ID, side);
		}
		else if (sig == SIGUSR2)
			pthread_exit((void*)0);
	}

	return (void*)0;
}

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


