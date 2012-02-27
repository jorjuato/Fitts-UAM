//////////////////////////////////////////////////////////////////
//  fitts.h
//
//	Cabeceras para definir parámetros que caracterizan
//	el experimento de Fitts
//
//
// Copyleft (L)		Marzo 2010		Jorge Ibáñez
//
//////////////////////////////////////////////////////////////////

#ifndef __INCLUDEDISPLAY
#include "display.h"
#define __INCLUDEDISPLAY
#endif

#include <math.h>

//////////////////////////////////////////////////////////////////
//		CONSTANTES 	DEL EXPERIMENTO
//////////////////////////////////////////////////////////////////

//TEMPORAL CONSTANTS
#define MIN_PRESSURE			5
#define TICK_INTERVAL  			9
#define LAST_CHANGE_INTERVAL	100
#define MAX_TRIAL_SAMPLES		100000
#define ONE_MILLION				1000000
#define INTER_TRIAL_TIME 		2000
#define ERROR_SCREEN_TIME 		1000

//SPATIAL CONSTANTS
#define AMPLITUDE				20.00
#define TARGET_WIDTH(ID)		(float) (2.0 * (float)(AMPLITUDE) / pow(2,(ID)))
#define TARGET_HEIGHT			20.00
#define TARGET_XMAX(ID)			(float)( ((float)(AMPLITUDE) / 2.0) + (float)((float)(TARGET_WIDTH(ID)) / 2.0) )
#define TARGET_XMIN(ID)			(float)( ((float)(AMPLITUDE) / 2.0) - (float)((float)(TARGET_WIDTH(ID)) / 2.0) )
#define TARGET_YMAX				(float)( (float)(TARGET_HEIGHT) / 2.0 )
#define TARGET_YMIN				(float) ( -(float)(TARGET_HEIGHT) / 2.0 )

//EXPERIMENTAL DESIGN CONSTANTS
#define ID_MIN					2.0
#define ID_MAX					3.0
#define ID_STEP					1.0
#define OSCILLATIONS_BEFORE		2
#define	OSCILLATIONS_DURING		6
#define OSCILLATIONS_AFTER		2
#define SLOPE_TIME				10.0
#define TRIAL_TYPES				(int)(( (ID_MAX) - (ID_MIN) )* 2.0 / (ID_STEP))
#define TRIAL_REPLICATIONS		1
#define SPEED_BUFFER_SIZE		5
#define POSITION_BUFFER_SIZE	7

//EXPERIMENT COLORS CONSTANTS
#define TARGET_COLOR			RGB_White
#define BG_NORMAL_COLOR			RGB_Black
#define BG_ERROR_COLOR			RGB_Red
#define BG_INTER_COLOR			RGB_Blue
#define BG_QUIT					RGB_Green


//////////////////////////////////////////////////////////////////
//		ESTRUCTURA DE DATOS PARA ALMACENAR CONFIGURACIONES
//////////////////////////////////////////////////////////////////

typedef enum{
	TRIAL_STATE_REC_ERROR,
	TRIAL_STATE_NOREC_ERROR,
	TRIAL_STATE_INTER_TRIAL,
	TRIAL_STATE_PRECONTACT,
	TRIAL_STATE_PREOSCILLATION,
	TRIAL_STATE_OSCILLATION_PRESLOPE,
	TRIAL_STATE_OSCILLATION_SLOPE,
	TRIAL_STATE_OSCILLATION_POSTSLOPE,
	TRIAL_STATE_QUITTING,
	TRIAL_STATE_MAX
} TRIALSTATE;

typedef struct {
	float x;
	float y;
	float vx;
	float vy;
	long int time;
	int   press;
	int oscillations;
	float ID;
} TRIALDATA;

typedef struct {
	float ID0;
	float IDf;
	char initSide;
	int replications;
} TRIALCONF;

typedef struct {
	float vx;
	float vy;
	long int t;
} SPEED;

typedef struct {
	float x;
	float y;
	long int t;
} POSITION;

typedef struct {
	float ID;
	char side;
	Uint8 state;
	Uint8 *bColor;
} GRAPHCONF;

//--------------------------------------------------------------//
//Definicion de verdad y falsedad, jeje
//--------------------------------------------------------------//
#define true 		1
#define false 		0
#define TRUE		true
#define FALSE		false
#define APPERROR	true
#define APPSUCCESS	false
#define ERROR		false
#define SUCCESS 	true



