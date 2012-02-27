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

#define TICK_INTERVAL  			10
#define LAST_CHANGE_INTERVAL	300
#define MAX_TRIAL_SAMPLES		100000
#define ONE_MILLION				1000000
#define INTER_TRIAL_TIME 		2000
#define ERROR_SCREEN_TIME 		1000
#define GRAPHICS_DELAY			200
#define SLOPE_TIME				30000.0

//SPATIAL CONSTANTS
#define MIN_PRESSURE			1
#define AMPLITUDE				20.00
#define TARGET_WIDTH(ID)		(float) (2.0 * (float)(AMPLITUDE) / pow(2,(ID)))
#define TARGET_HEIGHT			20.00
#define TARGET_XMAX(ID)			(float)( ((float)(AMPLITUDE) / 2.0) + (float)((float)(TARGET_WIDTH(ID)) / 2.0) )
#define TARGET_XMIN(ID)			(float)( ((float)(AMPLITUDE) / 2.0) - (float)((float)(TARGET_WIDTH(ID)) / 2.0) )
#define TARGET_YMAX				(float)( (float)(TARGET_HEIGHT) / 2.0 )
#define TARGET_YMIN				(float) ( -(float)(TARGET_HEIGHT) / 2.0 )
#define FEEDBACK_SIZE			0.2

//EXPERIMENTAL DESIGN CONSTANTS
#define ID_MIN					4.0
#define ID_MAX					6.0
#define ID_STEP					1.0
#define MIN_ID_STEP				0.002
#define ERROR_PERCENT			0.1
#define ERROR_MAX				15
#define	OSCILLATIONS_TRAIN		5
#define OSCILLATIONS_BEFORE		30
#define	OSCILLATIONS_DURING		6
#define OSCILLATIONS_AFTER		30
#define TRIAL_TYPES				(int)(( (ID_MAX) - (ID_MIN) )* 2.0 / (ID_STEP))
#define TRIAL_REPLICATIONS		10
#define SPEED_BUFFER_SIZE		11
#define POSITION_BUFFER_SIZE	13

//EXPERIMENT COLORS CONSTANTS
#define TARGET_COLOR			RGB_White
#define TARGET_2ND_COLOR		RGB_Gray
#define BG_NORMAL_COLOR			RGB_Black
#define BG_ERROR_COLOR			RGB_Red
#define BG_INTER_COLOR			RGB_Blue
#define BG_QUITTING				RGB_Green
#define FEEDBACK_COLOR			RGB_Red

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
	TRIAL_STATE_MAX
} TRIALSTATE;

typedef struct {
	float x;
	float y;
	float minx;
	float maxx;
	float vx;
	float vy;
	long int time;
	int   press;
	int oscillations;
	char side;
	float ID;
	int errors;
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
	//float x;
	//float y;
	char side;
	Uint8 state;
	long int last_change;
} GRAPHARG;

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



