//////////////////////////////////////////////////////////////////
//  main.c
//
//
// Copyleft (L)		Marzo 2010		Jorge Ibáñez
//
//////////////////////////////////////////////////////////////////


//Cabeceras estandar del SO
#include <sys/time.h>
#include <fcntl.h>
#include <time.h>

//Cabeceras específicas para configurar este experimento
#include "fitts.h"
#include "main.h"

//////////////////////////////////////////////////// GLOBALS

extern Uint8		g_trial_state;
extern TRIALCONF   	g_TrialConfig[(int)TRIAL_TYPES];
extern TRIALCONF 	g_config;
extern Uint8	 	g_trial_number;
extern char			g_datafile_path[128];
const char*			g_data_base="./data";
const char*			g_subject_name = NULL;




//------------------------------------------------------------  main()
//
int main(int argc, char **argv){
	OnInit(SDL_VIDEO_MODE);			//Inicializa recursos: sdlgl_control.c
	ExperimentSetup(argc,argv); 	//Configura el experimento
	InitGraphicsThread();			//Inicializa un thread para graficos: draw.c
	WacomInit(WACOM_SCANMODE_NOCALLBACK, NULL);
									//Inicializamos la tableta con la función handler: libwacom.a
	ExperimentLoop();				//Lanza el experimento propiamente dicho
	ExperimentEnd();				//Cierra archivos y libera recursos.
	return(0);						//No debería llegar nunca a este punto
}

//------------------------------------------------------------  ExperimentEnd()
//
static void ExperimentEnd(void){
	g_trial_state = TRIAL_STATE_END;
	EndGraphicsThread();
	WacomClose();
	exit(APPSUCCESS);
}

//------------------------------------------------------------  ExperimentLoop()
//
static void ExperimentLoop(){
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


//------------------------------------------------------------  ExperimentSetup()
//
static void ExperimentSetup(int argc, char **argv){
	Uint8 i=0,j=0;

	// Parsea la linea de comandos y el archivo de configuracion: 	auxfuncs.c
	ParseCmdLine(argc, argv);

	//Crea la mascara de eventos y escondemos cursor
	ExperimentEventMask();
	SDL_ShowCursor(FALSE);

	//Inicializamos la semilla del generador de numeros aleatorios
	srand(time(0));

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

//------------------------------------------------------------   int ParseCmdLine()
//
static void ParseCmdLine(int argc, char** argv) {
	const char* pa;

	++argv;
	if (argc == 1) {
		printf("Wrong calling convention\n");
		exit(0);
	}

	while ((pa = *(argv++)) != NULL) {
		if (!g_subject_name)
			g_subject_name = pa;
		else
			printf("Unknown argument %s\n",pa);
	}

	strcpy(g_datafile_path,g_data_base);
	strcat(g_datafile_path,"/");
	strcat(g_datafile_path,g_subject_name);
	mkdir(g_datafile_path, 0777);
	strcat(g_datafile_path,"/");
}

//------------------------------------------------------------  ExperimentEventMask()
//
static void ExperimentEventMask(){
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
