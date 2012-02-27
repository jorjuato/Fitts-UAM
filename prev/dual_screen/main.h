//////////////////////////////////////////////////////////////////
//  main.h
//
//
// Copyleft (L)		Marzo 2010		Jorge Ibáñez
//
//////////////////////////////////////////////////////////////////


//Cabeceras estandar del SO
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

//Cabeceras con definiciones para creacion de hilos
#include <pthread.h>

//Cabeceras de la libreria SDL
#ifndef __INCLUDESDL
#include "SDL/SDL.h"
#define __INCLUDESDL
#endif

//Cabeceras con definiciones del API Wacom
#ifndef __INCLUDEWACOM
#include "wacom.h"
#define __INCLUDEWACOM
#endif

//Cabeceras específicas para configurar este experimento
#include "fitts.h"
#include "draw.h"

//Cabeceras genéricas para todos los experimentos
#include "display.h"
#include "auxfuncs.h"
#include "sdlgl_control.h"

#define APP_NAME	"SDL Fitts Experiment"
#define APP_VERSION	"0.0.0.0.0"

//////////////////////////////////////////////////////////////////
//		PROTOTIPOS DE LAS FUNCIONES								//
//////////////////////////////////////////////////////////////////

void OnExit();

void OnInit();

static void ExperimentLoop(void);

static int ExperimentFSA();

static void ExperimentGraphics();

static void* _ExperimentGraphics(void*);

static void ExperimentEnd(void);

void printState();



