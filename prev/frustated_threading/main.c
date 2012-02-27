//////////////////////////////////////////////////////////////////
//  main.c
//
//
// Copyleft (L)		Marzo 2010		Jorge Ibáñez
//
//////////////////////////////////////////////////////////////////

//Cabeceras estandar del SO
#include <time.h>


//Cabeceras con definiciones del API Wacom
#ifndef __INCLUDEWACOM
#include "wacom.h"
#define __INCLUDEWACOM
#endif

//Cabeceras específicas para configurar este experimento
#include "sdl.h"
#include "fitts.h"
#include "fsa.h"
#include "main.h"

#define APP_NAME	"SDL Fitts Experiment"
#define APP_VERSION	"0.0.0.0.0"



//////////////////////////////////////////////////// GLOBALS
extern TRIALCONF   	g_TrialConfig[(int)TRIAL_TYPES];
extern Uint8		g_trial_state;
/////////////////////////////////////////////////////////////


//------------------------------------------------------------  main()
//
int main(int argc, char **argv){

	OnInit(SDL_VIDEO_MODE);			//Inicializa recursos: sdl.c
	ExperimentSetup(argc,argv); 	//Configura el experimento
	printf("SETUP\n");
	fflush(stdout);
	ExperimentLoop();				//Lanza el experimento propiamente dicho
	ExperimentEnd();				//Cierra archivos y libera recursos.
	return(0);						//No debería llegar nunca a este punto
}

//------------------------------------------------------------  EndExperiment()
//
static void ExperimentEnd(void){
	printf("Cerrando el programa y liberando recursos\n");
	fflush(stdout);
	g_trial_state = TRIAL_STATE_QUITTING;
	ExperimentGraphics();
	WacomClose();
	exit(APPSUCCESS);
}

//------------------------------------------------------------  MainLoop()
//
static void ExperimentLoop()
{
    SDL_Event event;

	//Bucle indefinido que lleva a cabo el experimento
    while(TRUE)
    {
        //Implementacion del timer sin usar señales
		SetDelay(TICK_INTERVAL*ONE_MILLION);

        //Funcion principal para regular los estados del experimento
        //Contiene los condicionales asociados a la salida de WACOMlib
    	if (!ExperimentFSA())
    	 	break;

        //Bucle principal de eventos SDL
        //Si no se va a obtener ningún evento útil, mejor desecharlos...
        //Ahora solo procesa el escape antes de tocar la pantalla
        while( SDL_PollEvent(&event))
       		if (g_trial_state == TRIAL_STATE_PRECONTACT)
				if (event.key.keysym.sym == SDLK_ESCAPE || event.type == SDL_QUIT)
					return;
    }
}

//------------------------------------------------------------  SetDelay()
//
static void SetDelay(long int ns){
	struct timespec ts_req, ts_rem;
	ts_req.tv_sec = 0;
	ts_req.tv_nsec = ns;
	/* Recursive call in case of signal reception*/
	if (nanosleep(&ts_req, &ts_rem)==-1)
		SetDelay(ts_rem.tv_nsec);
}

//------------------------------------------------------------  ExperimentSetup()
//
void ExperimentSetup(int argc, char **argv){
	Uint8 i=0,j=0;


	// Parsea la linea de comandos y el archivo de configuracion: 	auxfuncs.c
	ParseCmdLine(argc, argv);
	printf("Wacoming\n");
	fflush(stdout);
	//Inicializamos la tableta con la función handler				libwacom.a
	WacomInit(WACOM_SCANMODE_NOCALLBACK, NULL);

	//Crea la mascara de eventos y escondemos cursor
	ExperimentEventMask();
	SDL_ShowCursor(FALSE);

	//Inicializamos la semilla del generador de numeros aleatorios
	srand(time(0));
	printf("Set conf array\n");
	fflush(stdout);
	//Initialize Configurations array
	for(i=ID_MIN;i<ID_MAX;i+=ID_STEP){
		g_TrialConfig[j].ID0 = i;
		g_TrialConfig[j].IDf = i+ID_STEP;
		//g_TrialConfig[j].initSide = (rand()/((double)RAND_MAX + 1) > 0.5) ? 'L' : 'R';
		g_TrialConfig[j].replications = TRIAL_REPLICATIONS;
		j++;
	}
	for(i=ID_MAX;i>ID_MIN;i-=ID_STEP){
		g_TrialConfig[j].ID0 = i;
		g_TrialConfig[j].IDf = i-ID_STEP;
		//g_TrialConfig[j].initSide = (rand()/((double)RAND_MAX + 1) > 0.5) ? 'L' : 'R';
		g_TrialConfig[j].replications = TRIAL_REPLICATIONS;
		j++;
	}
}

//------------------------------------------------------------  ExperimentEventMask()
//
void ExperimentEventMask(){
	// Ignorar todos los eventos que no van a usarse
	SDL_EventState(SDL_KEYUP, SDL_IGNORE);
	SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);
	SDL_EventState(SDL_JOYAXISMOTION, SDL_IGNORE);
	SDL_EventState(SDL_JOYBALLMOTION, SDL_IGNORE);
	SDL_EventState(SDL_JOYHATMOTION, SDL_IGNORE);
	SDL_EventState(SDL_JOYBUTTONDOWN, SDL_IGNORE);
	SDL_EventState(SDL_JOYBUTTONUP, SDL_IGNORE);
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_VIDEORESIZE, SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);
}

