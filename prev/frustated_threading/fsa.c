//////////////////////////////////////////////////////////////////
//  fsa.c
//
//	Finite State Automatons (FSA) que controlan el devenir de los estados
//  en el experimento y las acciones asociadas a cada uno de ellos.
//
//
//  Copyleft (L)		Mayo 2010		Jorge Ibáñez
//
//////////////////////////////////////////////////////////////////

//Cabeceras de la libreria Wacom
#ifndef __INCLUDEWACOM
#include "wacom.h"
#define __INCLUDEWACOM
#endif

//Cabeceras estandar del SO
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>

//Cabeceras principales del experimento
#include "fitts.h"
#include "draw.h"
#include "fsa.h"

//////////////////////////////////////////////////// EXPERIMENT GLOBALS
Uint32		g_init_time = 0;
Uint32		g_data_count = 0;
Uint8 		g_trial_number=0;
Sint8		g_trial_type = 0;
Uint8		g_trial_state = TRIAL_STATE_INTER_TRIAL;
Uint32		g_slope_time = 0;
char		g_side='L';
float 		g_ID=0;
Uint8 		g_errors = 0;
TRIALDATA	g_TrialData[MAX_TRIAL_SAMPLES];
TRIALCONF   g_TrialConfig[(int)TRIAL_TYPES], g_config;
POSITION 	g_PositionBuffer[POSITION_BUFFER_SIZE];
SPEED 		g_SpeedBuffer[SPEED_BUFFER_SIZE];


//////////////////////////////////////////////////// I/O GLOBALS
const char*		g_subject_name = NULL;
const char*		g_data_base="./data";
char			g_datafile_path[128];


//////////////////////////////////////////////////// STATIC DEFINITIONS
static int ExperimentSelect();

static void ExperimentSaveData();

static Uint8 IsInTarget(float x, float y);

static Uint8 UpdateSpeedBuffers(float* vx, float* vy, long int now);

static long int getmicroseconds();

static void itoa(int value, char* str, int base);

static void strreverse(char* begin, char* end);



//------------------------------------------------------------  ExperimentGraphics()
//
void ExperimentGraphics() {
	pthread_t graphThread;
	pthread_attr_t attr;
	static GRAPHCONF conf;
	static Uint8 bColor=0;

	conf.ID = g_ID;
	conf.side = g_side;
	conf.state = g_trial_state;
	conf.bColor = &bColor;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if (pthread_create(&graphThread, &attr, GraphicsFSA, (void*)&conf))
		ExperimentGraphics();
}

//------------------------------------------------------------  ExperimentTimer()
//
int ExperimentFSA(void){

	long int now,time_inc;
	static long int last_change = 0;
	Uint16 press;
	Uint8 change=0,i=0;
	float vx=0, vy=0, IDtmp;
	static Uint8 counter = 0;
	static Uint8 bRepeat = FALSE;
	static Uint8 oscillations = 0, slopeOsc=0;
	static float x,y;

	//Actualizamos variables temporales
	now = getmicroseconds();
	time_inc = now - g_init_time;

	//Actualizamos los variables de la tableta
	x     = WAC_ABS_SCALE_X(gAbsState[WACOMFIELD_POSITION_X].nValue);
	y     = WAC_ABS_SCALE_Y(gAbsState[WACOMFIELD_POSITION_Y].nValue);
	press = gAbsState[WACOMFIELD_PRESSURE].nValue;

	//Actualizamos el buffer de posiciones
	for(i=POSITION_BUFFER_SIZE-1;i>0;i--)
		g_PositionBuffer[i] = g_PositionBuffer[i-1];
	g_PositionBuffer[0].x = x;
	g_PositionBuffer[0].y = y;
	g_PositionBuffer[0].t = now;

	//Actualizamos el buffer de velocidades

	if ( (change = UpdateSpeedBuffers(&vx, &vy, now)) ) {
		if (now - last_change < LAST_CHANGE_INTERVAL*1000)
			change = FALSE;
		last_change =  now;
	}

	// Guardamos estado detallado del movimiento si procede
	if (g_trial_state > TRIAL_STATE_PRECONTACT){
		g_TrialData[g_data_count].x = x;
		g_TrialData[g_data_count].y = y;
		g_TrialData[g_data_count].vx = vx;
		g_TrialData[g_data_count].vy = vy;
		g_TrialData[g_data_count].time = time_inc;
		g_TrialData[g_data_count].press = press;
		g_TrialData[g_data_count].oscillations = oscillations;
		g_TrialData[g_data_count].ID = g_ID;
		g_data_count++;

		//printf("x = %f, y = %f\t", x, y);
		//printf("vx = %0.10f, vy = %0.10f time_inc = %ld, ID = %f\n", vx, vy, time_inc, g_ID);
		//printState();
		//fflush(stdout);
	}

	//Automata de estados, bifurca el flujo del programa
	switch(g_trial_state)
	{
		case TRIAL_STATE_REC_ERROR:///////////////////////////////////////////
		case TRIAL_STATE_NOREC_ERROR:
			sleep(ERROR_SCREEN_TIME/1000);
			g_trial_state = TRIAL_STATE_INTER_TRIAL;
			g_TrialConfig[g_trial_type].replications += 1;
			ExperimentGraphics();
			break;
		case TRIAL_STATE_INTER_TRIAL:///////////////////////////////////
			if (!bRepeat) {
				if (!ExperimentSelect()) return FALSE;
				ExperimentGraphics();
				bRepeat = TRUE;
				g_init_time = now;
				g_errors = 0;
				slopeOsc=0;
			}
			else if (time_inc >= INTER_TRIAL_TIME*1000)	{
				g_trial_state = TRIAL_STATE_PRECONTACT;
				bRepeat = FALSE;
				ExperimentGraphics();
			}
			break;
		case TRIAL_STATE_PRECONTACT:////////////////////////////////////
			if (press > MIN_PRESSURE){
				if (IsInTarget(x,y)) {
					g_trial_state = TRIAL_STATE_PREOSCILLATION;
					oscillations = 0;
					ExperimentGraphics();
				}
			}
			else if (counter > 100) {
				ExperimentGraphics();
				counter=0;
				}
			else
				counter++;
			break;
		case TRIAL_STATE_PREOSCILLATION:////////////////////////////////
			if (press > MIN_PRESSURE) {
				if (fabs(vx) > 0) { // Check for non null speed as trial init
					g_trial_state = TRIAL_STATE_OSCILLATION_PRESLOPE;
					g_init_time = now;
					ExperimentGraphics();
				}
			}
			else {
				g_trial_state = TRIAL_STATE_REC_ERROR;
				ExperimentGraphics();
			}
			break;
		case TRIAL_STATE_OSCILLATION_PRESLOPE://////////////////////////
			if (press > MIN_PRESSURE) {
				if (change){
					//printf("Pues, ha cambiado!!\n");
					//fflush(stdout);
					if (IsInTarget(x,y)){
						//printf("Y encima esta en el target %f %f \n", x, y);
						g_side = (g_side == 'R') ? 'L' : 'R';
						oscillations++;
						//printf("Numero de oscilaciones = %d\n",oscillations);
						//fflush(stdout);
					}
					else{
						g_errors++;
						//printf("Numero de errores = %d",g_errors);
						//fflush(stdout);
					}
				}
				if (oscillations >= OSCILLATIONS_BEFORE) {
					g_trial_state = TRIAL_STATE_OSCILLATION_SLOPE;
					g_slope_time = now;
					ExperimentGraphics();
				}
			}
			else {
				g_trial_state = TRIAL_STATE_REC_ERROR;
				ExperimentGraphics();
			}
			break;
		case TRIAL_STATE_OSCILLATION_SLOPE://///////////////////////////
			if (press > MIN_PRESSURE) {
				if (now - g_slope_time >= SLOPE_TIME*ONE_MILLION){
				//if (oscillations >= OSCILLATIONS_BEFORE + OSCILLATIONS_DURING) {
					g_trial_state = TRIAL_STATE_OSCILLATION_POSTSLOPE;
					g_ID = g_config.IDf; //Shouldn't be necessary
					oscillations = 0;
					ExperimentGraphics();
				}
				else {
					if (change){
						printf("Pues, ha cambiado!!\n");
						fflush(stdout);
						if (IsInTarget(x,y)){
							printf("Y encima esta en el target %f %f \n", x, y);
							g_side = (g_side == 'R') ? 'L' : 'R';
							slopeOsc++;
							printf("Numero de oscilaciones = %d\n",slopeOsc);
							fflush(stdout);
							//g_ID = (g_config.IDf-g_config.ID0)/OSCILLATIONS_DURING * (oscillations-OSCILLATIONS_BEFORE)+ g_config.ID0;
						}
						else {
							g_errors++;
							printf("Numero de errores = %d\n",g_errors);
							fflush(stdout);
						}
					}
					IDtmp = (float)(g_config.IDf-g_config.ID0)/(float)SLOPE_TIME * (float)((float)(now - g_slope_time) / (float)ONE_MILLION) + g_config.ID0;
					//printf("ID - IDtmp = %f \n",g_ID - IDtmp );
					if (fabs(g_ID - IDtmp) > 0.01) {
						g_ID = IDtmp;
						ExperimentGraphics();
					}
					//printf("ID=%f\t\n",g_ID);
					//fflush(stdout);
					//printf("IncID=%f \t slope = %f\n",(g_config.IDf-g_config.ID0), (g_config.IDf-g_config.ID0)/SLOPE_TIME );
					//printf("IncTime = %f \t seconds = %f\n", (float)(now - g_slope_time),(float)(now - g_slope_time) / (float)ONE_MILLION);
				}
			}
			else {
				g_trial_state = TRIAL_STATE_REC_ERROR;
				ExperimentGraphics();
			}
			break;
		case TRIAL_STATE_OSCILLATION_POSTSLOPE://///////////////////////
			if (press > MIN_PRESSURE){
				if (change){
					//printf("Pues, ha cambiado!!\n");
					if (IsInTarget(x,y)){
						//printf("Y encima esta en el target %f %f \n", x, y);
						g_side = (g_side == 'R') ? 'L' : 'R';
						oscillations++;
						//printf("Numero de oscilaciones = %d\n",oscillations);
						//fflush(stdout);
					}
					else{
						g_errors++;
						//printf("Numero de errores = %d",g_errors);
						//fflush(stdout);
					}
				}
				if (oscillations >=  OSCILLATIONS_AFTER) {
					if (g_errors < ((float)(OSCILLATIONS_BEFORE + slopeOsc +OSCILLATIONS_AFTER) / 20.0) ) {
					//if (TRUE){
						ExperimentSaveData(); //Add errors and oscilation to file header
						g_trial_state = TRIAL_STATE_INTER_TRIAL;
						oscillations = 0;
						g_data_count = 0;
						g_errors = 0;
						ExperimentGraphics();
					}
					else {
						printf("Too much erroros: %d\n", g_errors);
						fflush(stdout);
						g_trial_state = TRIAL_STATE_REC_ERROR;
						ExperimentGraphics();
					}
				}
			}
			else {
				g_trial_state = TRIAL_STATE_REC_ERROR;
				ExperimentGraphics();
			}
			break;
        case TRIAL_STATE_QUITTING://///////////////////////////////////////
		default:
			return FALSE;
			break;
	} //end_switch
	return TRUE;
}	//end_function

//------------------------------------------------------------  ExperimentFSA()
//
static void *GraphicsFSA(void *conf) {
	float ID 		= ((GRAPHCONF *) conf)->ID;
	char  side 		= ((GRAPHCONF *) conf)->side;
	Uint8 state 	= ((GRAPHCONF *) conf)->state;
	Uint8 *bColor	= ((GRAPHCONF *) conf)->bColor;

	switch(state)
	{
		case TRIAL_STATE_INTER_TRIAL:///////////////////////////////////////////
			ShowBackground(BG_INTER_COLOR);
			//ShowMessage();
			break;
		case TRIAL_STATE_PRECONTACT:///////////////////////////////////////////
			ShowBackground(BG_NORMAL_COLOR);
			if (*bColor){
				*bColor = 0;
				if (side == 'L')
					ShowTargets(RGB_Gray,RGB_White,ID);
				else
					ShowTargets(RGB_White,RGB_Gray,ID);
			}
			else {
				*bColor = 1;
				ShowTargets(RGB_White,RGB_White,ID);
			}
			break;
		case TRIAL_STATE_PREOSCILLATION:///////////////////////////////////////////
		case TRIAL_STATE_OSCILLATION_PRESLOPE:
		case TRIAL_STATE_OSCILLATION_SLOPE:
		case TRIAL_STATE_OSCILLATION_POSTSLOPE:
			ShowBackground(BG_NORMAL_COLOR);
			ShowTargets(RGB_White,RGB_White,ID);
			break;

		case TRIAL_STATE_REC_ERROR://///////////////////////////////////////
			ShowBackground(BG_ERROR_COLOR);
			//ShowMessage(g_error_msg);
			break;

		case TRIAL_STATE_NOREC_ERROR://///////////////////////////////////////
			ShowBackground(BG_ERROR_COLOR);
			//ShowMessage(g_error_msg);
			break;

		case TRIAL_STATE_QUITTING://///////////////////////////////////////
			ShowBackground(BG_QUIT);
			//ShowMessage(g_error_msg);
			break;

		default://////////////////////////////////////////////////////////////
			exit(APPERROR);
			break;
	}
	UpdateScreen();
	pthread_exit(NULL);
}

//------------------------------------------------------------  ExperimentFSA()
//
void printState(){
	switch(g_trial_state){
		case TRIAL_STATE_INTER_TRIAL:
			printf("TRIAL_STATE_INTER_TRIAL\n");
			break;
		case TRIAL_STATE_PRECONTACT:
			printf("TRIAL_STATE_PRECONTACT\n");
			break;
		case TRIAL_STATE_PREOSCILLATION:
			printf("TRIAL_STATE_PREOSCILLATION\n");
			break;
		case TRIAL_STATE_OSCILLATION_PRESLOPE:
			printf("TRIAL_STATE_OSCILLATION_PRESLOPE\n");
			break;
		case TRIAL_STATE_OSCILLATION_SLOPE:
			printf("TRIAL_STATE_OSCILLATION_SLOPE\n");
			break;
		case TRIAL_STATE_OSCILLATION_POSTSLOPE:
			printf("TRIAL_STATE_OSCILLATION_POSTSLOPE\n");
			break;
		case TRIAL_STATE_REC_ERROR:
			printf("TRIAL_STATE_REC_ERROR\n");
			break;
		case TRIAL_STATE_NOREC_ERROR:
			printf("TRIAL_STATE_NOREC_ERROR\n");
			break;
        case TRIAL_STATE_QUITTING:
			printf("TRIAL_STATE_QUITTING\n");
			break;
		default:
			printf("default\n");
			break;
		fflush(stdout);
	}
}


/////////////////////////////////////////////////////////////////////////////
////////////////        FSA Auxiliar Functions    ///////////////////////////
/////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------  ExperimentSelect()
//
static int ExperimentSelect(){
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

	return TRUE;
}

//------------------------------------------------------------  ExperimentSaveData()
//
static void ExperimentSaveData(){
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
	fprintf(datafile, "# x \t y \t vx \t vy \t press \t time \t oscillations \t ID\n");

	// Save temporal series
	for(i=0;i<g_data_count;i++)
		fprintf(datafile,"%f\t%f\t%f\t%f\t%d\t%ld\t%d\t%f\n",\
				g_TrialData[i].x, g_TrialData[i].y, g_TrialData[i].vx, g_TrialData[i].vy,\
				g_TrialData[i].press, g_TrialData[i].time, g_TrialData[i].oscillations, g_TrialData[i].ID);
	fclose(datafile);
}

//------------------------------------------------------------  itoa()
//Ansi C "itoa" based on Kernighan & Ritchie's "Ansi C":
static void strreverse(char* begin, char* end){
	char aux;
	while(end>begin)
		aux=*end, *end--=*begin, *begin++=aux;
}
static void itoa(int value, char* str, int base) {
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


//------------------------------------------------------------  getmicroseconds()
//
static long int getmicroseconds() {
	static struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_usec + tv.tv_sec*ONE_MILLION;
}

//------------------------------------------------------------  IsInTarget(g_side, ID, x, y)
//
static Uint8 IsInTarget(float x, float y) {
	if (g_side == 'L'){
		if ( (x > -TARGET_XMAX(g_ID)) && (x < -TARGET_XMIN(g_ID)) &&  (y < TARGET_YMAX) && (y > TARGET_YMIN) )
			return TRUE;
	}
	else {
		if ( (x < TARGET_XMAX(g_ID)) && (x > TARGET_XMIN(g_ID)) &&  (y < TARGET_YMAX) && (y > TARGET_YMIN) )
			return TRUE;
	}
	return FALSE;
}

//------------------------------------------------------------  UpdateSpeedBuffers(float* vx, float* vy, long int now)
//
static Uint8 UpdateSpeedBuffers(float* vx, float* vy, long int now){
	int i,j;

	//Actualizamos el buffer de velocidades
	for(i=SPEED_BUFFER_SIZE-1;i>0;i--) {
		g_SpeedBuffer[i] = g_SpeedBuffer[i-1];
		//*vx = *vx + g_SpeedBuffer[i].vx;
		//*vy = *vy + g_SpeedBuffer[i].vy;
	}
	g_SpeedBuffer[0].vx = (g_PositionBuffer[0].x-g_PositionBuffer[1].x)/(g_PositionBuffer[0].t-g_PositionBuffer[1].t) * ONE_MILLION;
	g_SpeedBuffer[0].vy = (g_PositionBuffer[0].y-g_PositionBuffer[1].y)/(g_PositionBuffer[0].t-g_PositionBuffer[1].t) * ONE_MILLION;
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


//------------------------------------------------------------   int ParseCmdLine()
//
void ParseCmdLine(int argc, char** argv) {
	const char* pa;

	++argv;
	if (argc == 1) {
		printf("Wrong calling convention\n");
		exit(APPERROR);
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
