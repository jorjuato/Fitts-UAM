/************************************************************************##
** 	auxfuncs.c     									 			 		 ##
**																		 ##
**			Copyleft (L)		Marzo 2010		Jorge Ibáñez			 ##
**																		 ##
**																		 ##
**																		 ##
**  Conjunto de funciones auxiliares para el programa principal del 	 ##
**  experimento. Controlan entrada y salida de ficheros y cuestiones	 ##
**  secundarias del algoritmo que complican la lectura de main.c		 ##
**																		 ##
**************************************************************************/

//Cabeceras de la libreria SDL
#ifndef __INCLUDESDL
#include "SDL/SDL.h"
#define __INCLUDESDL
#endif

//Cabeceras de la libreria Wacom
#ifndef __INCLUDEWACOM
#include "wacom.h"
#define __INCLUDEWACOM
#endif

//Cabeceras estandar del SO
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
//#include <unistd.h>
//#include <math.h>
#include <sys/time.h>


//Cabeceras principales del experimento
#include "display.h"
#include "fitts.h"
#include "auxfuncs.h"

//////////////////////////////////////////////////// EXTERN GLOBAL DEFINITIONS
extern Sint8	   	g_trial_type;
extern Uint32		g_data_count;
extern char			g_side;
extern float		g_ID;
extern TRIALDATA	g_TrialData[MAX_TRIAL_SAMPLES];
extern TRIALCONF  	g_TrialConfig[TRIAL_TYPES];
extern TRIALCONF  	g_config;
extern POSITION		g_PositionBuffer[POSITION_BUFFER_SIZE];
extern SPEED 		g_SpeedBuffer[SPEED_BUFFER_SIZE];

//////////////////////////////////////////////////// I/O GLOBALS
const char*		g_subject_name = NULL;
const char*		g_data_base="./data";
char			g_datafile_path[128];
Uint8	 		g_trial_number=0;


//------------------------------------------------------------  ExperimentSetup()
//
void ExperimentSetup(int argc, char **argv){
	Uint8 i=0,j=0;

	// Parsea la linea de comandos y el archivo de configuracion: 	auxfuncs.c
	ParseCmdLine(argc, argv);

	//Inicializamos la tableta con la función handler				libwacom.a
	WacomInit(WACOM_SCANMODE_NOCALLBACK, NULL);

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

	//Initialize speed and position buffers to zero
	for (i=0;i<SPEED_BUFFER_SIZE;i++) {
		g_SpeedBuffer[i].vx=0;
		g_SpeedBuffer[i].vy=0;
		g_SpeedBuffer[i].t=0;
	}
	for (i=0;i<POSITION_BUFFER_SIZE;i++){
		g_PositionBuffer[i].x=0;
		g_PositionBuffer[i].y=0;
		g_PositionBuffer[i].t=0;
	}
}

//------------------------------------------------------------   int ParseCmdLine()
//
void ParseCmdLine(int argc, char** argv) {
	const char* pa;

	++argv;
	if (argc == 1)
		printf("Wrong calling convention\n");

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

//------------------------------------------------------------  ExperimentSelect()
//
int ExperimentSelect(){
	double proporciones[TRIAL_TYPES+1],rnd_num;
	int i, acumulated_trials = 0, total_trials = 0;

	//Sumamos el numero total de trials restantes
	for (i=0;i<TRIAL_TYPES;i++)	{
		total_trials += g_TrialConfig[i].replications;
		proporciones[i] = 0;
	}

	//Comprobamos que quedan trials por realizar
	printf("Number of trials left: %d\n", total_trials);
	fflush(stdout);
	if (total_trials == 0)
		return FALSE;

	//Obtenemos las proporciones de cada trial_type
	for (i=1;i<TRIAL_TYPES+1;i++) {
		acumulated_trials += g_TrialConfig[i-1].replications;
		proporciones[i] = (float) acumulated_trials / (float)total_trials; //(float)
		printf("Trial proportions: %f\n", proporciones[i]);
		fflush(stdout);
	}
	//Ahora, elegimos aleatoriamente el tipo de trial
	g_trial_type = -1;
	do {
		rnd_num = (float)rand() / (float)(RAND_MAX+1.0);
		printf("el número aleatorio obtenido es...: %f\n", rnd_num);
		for (i=0;i<TRIAL_TYPES;i++)
			if( rnd_num > proporciones[i] && rnd_num<proporciones[i+1])
				if (g_TrialConfig[i].replications>0)
					g_trial_type = i;
	} while(g_trial_type == -1);
	g_TrialConfig[g_trial_type].replications -= 1;
	g_config = g_TrialConfig[g_trial_type];
	g_trial_number++;
	g_ID = g_config.ID0;
	g_config.initSide = (rand()/((double)RAND_MAX + 1) > 0.5) ? 'L' : 'R';
	g_side = g_config.initSide;
	return TRUE;
}

//------------------------------------------------------------  ExperimentSaveData()
//
void ExperimentSaveData(){
	//char* trial_number, filename;
	Uint8 charlen;
	Uint32 i;
	FILE* datafile;
	char trial_number[8];
	char filename[128];

	// Compose filename
	itoa(g_trial_number,trial_number,10);
	charlen = strlen(trial_number);
	strcpy(filename,g_datafile_path);
	for (i=0;i<4-charlen;i++)
		strcat(filename,"0");
	strcat(filename,trial_number);
	strcat(filename,".dat");
	datafile = fopen(filename, "w");

	// Save header
	fprintf(datafile, "ID0=%1.2f \t IDf=%1.2f \t initSide=%s \t OBS=%d \t ODS=%d \t OAS=%d \t SlopeTime= %f \t replications=%d\n",\
			g_config.ID0, g_config.IDf, &g_config.initSide, OSCILLATIONS_BEFORE,OSCILLATIONS_DURING, OSCILLATIONS_AFTER, SLOPE_TIME, g_config.replications );
	fprintf(datafile, "# x \t\t MinX \t\t MaxX \t\t y \t\t\t vx \t\tvy\t\t\tpress\ttime\t\tside\tosc\terrors\tID\n");

	// Save temporal series
	for(i=0;i<g_data_count;i++)
		fprintf(datafile,"%2.2f\t\t%2.2f\t\t%2.2f\t\t%2.2f\t\t%2.2f\t\t%2.2f\t\t%d\t\t%5.1f\t\t%c\t\t%d\t\t%d\t\t%2.3f\n",\
				g_TrialData[i].x, g_TrialData[i].minx, g_TrialData[i].maxx,\
				g_TrialData[i].y, g_TrialData[i].vx, g_TrialData[i].vy,\
				g_TrialData[i].press, (float)g_TrialData[i].time/1000.0, g_TrialData[i].side, \
				g_TrialData[i].oscillations, g_TrialData[i].errors, g_TrialData[i].ID);
	fclose(datafile);
}

//------------------------------------------------------------  itoa()
//Ansi C "itoa" based on Kernighan & Ritchie's "Ansi C":
void strreverse(char* begin, char* end){
	char aux;
	while(end>begin)
		aux=*end, *end--=*begin, *begin++=aux;
}
void itoa(int value, char* str, int base) {
	static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char* wstr=str;
	int sign;

	// Validate base
	if (base<2 || base>35){ *wstr='\0'; return; }
	// Take care of sign
	if ((sign=value) < 0) value = -value;
	// Conversion. Number is reversed.
	do *wstr++ = num[value%base]; while(value/=base);
	if(sign<0) *wstr++='-';
	*wstr='\0';
	// Reverse string
	strreverse(str,wstr-1);
}


//------------------------------------------------------------  SetDelay()
//
void SetDelay(long int ns){
	struct timespec ts_req, ts_rem;
	ts_req.tv_sec = 0;
	ts_req.tv_nsec = ns;
	/* Recursive call in case of signal reception*/
	if (nanosleep(&ts_req, &ts_rem)==-1)
		SetDelay(ts_rem.tv_nsec);
}

//------------------------------------------------------------  getmicroseconds()
//
long int getmicroseconds() {
	static struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_usec + tv.tv_sec*ONE_MILLION;
}

//------------------------------------------------------------  IsInTarget(g_side, g_ID, x, y)
//
Uint8 IsInTarget(float x, float y) {
	if (g_side == 'L'){
		if ( (x >= -TARGET_XMAX(g_ID)) && (x <= -TARGET_XMIN(g_ID)) &&  (y <= TARGET_YMAX) && (y >= TARGET_YMIN) )
			return TRUE;
	}
	else {
		if ( (x <= TARGET_XMAX(g_ID)) && (x >= TARGET_XMIN(g_ID)) &&  (y <= TARGET_YMAX) && (y >= TARGET_YMIN) )
			return TRUE;
	}
	return FALSE;
}

//------------------------------------------------------------  minx()
//
float minx(){
	if (g_side == 'L')
		return -TARGET_XMAX(g_ID);
	else
		return TARGET_XMIN(g_ID);
}

//------------------------------------------------------------  maxx()
//
float maxx(){
	if (g_side == 'L')
		return -TARGET_XMIN(g_ID);
	else
		return TARGET_XMAX(g_ID);
}

Uint8 UpdateSpeedBuffers(float* vx, float* vy, long int now){
	int i,j;

	//Actualizamos el buffer de velocidades
	for(i=SPEED_BUFFER_SIZE-1;i>0;i--) {
		g_SpeedBuffer[i] = g_SpeedBuffer[i-1];
		//*vx = *vx + g_SpeedBuffer[i].vx;
		//*vy = *vy + g_SpeedBuffer[i].vy;
	}
	g_SpeedBuffer[0].vx = (g_PositionBuffer[0].x-g_PositionBuffer[1].x)/(g_PositionBuffer[0].t-g_PositionBuffer[1].t) * ONE_MILLION; // + g_SpeedBuffer[1].vx)/2;
	g_SpeedBuffer[0].vy = (g_PositionBuffer[0].y-g_PositionBuffer[1].y)/(g_PositionBuffer[0].t-g_PositionBuffer[1].t) * ONE_MILLION; // + g_SpeedBuffer[1].vy)/2;
	g_SpeedBuffer[0].t = now;

	//Calculamos la velocidad media a apartir de las últimas SPEED_BUFFER_SIZE velocidades instantáneas.
	//*vx = (*vx + g_SpeedBuffer[0].vx)/ SPEED_BUFFER_SIZE;
	//*vy = (*vy + g_SpeedBuffer[0].vy)/ SPEED_BUFFER_SIZE;
	*vx = g_SpeedBuffer[0].vx;
	*vy = g_SpeedBuffer[0].vy;

	//Calculamos cambios de signo en la velocidad en x (cambio de dirección del movimiento)
	for(i=0;i<SPEED_BUFFER_SIZE;i++)
		for(j=SPEED_BUFFER_SIZE-1;j>=0;j--)
			if (g_SpeedBuffer[i].vx*g_SpeedBuffer[j].vx < 0)
				return TRUE;
	return FALSE;
}
