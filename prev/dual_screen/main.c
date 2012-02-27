//////////////////////////////////////////////////////////////////
//  main.c
//
//
// Copyleft (L)		Marzo 2010		Jorge Ibáñez
//
//////////////////////////////////////////////////////////////////

#include "main.h"

Uint32		g_init_time = 0;
Uint32		g_data_count = 0;
Sint8		g_trial_type = 0;
Uint8		g_trial_state = TRIAL_STATE_INTER_TRIAL;
Uint32		g_slope_time = 0;
long int 	g_last_change = 0;
char		g_side;
float 		g_ID=0;
Uint8 		g_errors = 0;
TRIALDATA	g_TrialData[MAX_TRIAL_SAMPLES];
TRIALCONF   g_TrialConfig[(int)TRIAL_TYPES], g_config;
POSITION 	g_PositionBuffer[POSITION_BUFFER_SIZE];
SPEED 		g_SpeedBuffer[SPEED_BUFFER_SIZE];
float 		x,y;

//------------------------------------------------------------  main()
//
int main(int argc, char **argv){

	OnInit(SDL_VIDEO_MODE);			//Inicializa recursos: sdlgl_control.c
	ExperimentSetup(argc,argv); 	//Configura el experimento: auxfuncs.c
	ExperimentLoop();				//Lanza el experimento propiamente dicho
	ExperimentEnd();				//Cierra archivos y libera recursos.
	return(0);						//No debería llegar nunca a este punto
}

//------------------------------------------------------------  EndExperiment()
//
static void ExperimentEnd(void){
	printf("Cerrando el programa y liberando recursos\n");
	fflush(stdout);
	ShowBackground(BG_QUITTING);
	UpdateScreen();
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

		ExperimentGraphics();

        //Bucle principal de eventos SDL
        //Si no se va a obtener ningún evento útil, mejor desecharlos...
        //Ahora solo procesa el escape antes de tocar la pantalla
        while( SDL_PollEvent(&event))
       		if (g_trial_state == TRIAL_STATE_PRECONTACT)
				if (event.key.keysym.sym == SDLK_ESCAPE || event.type == SDL_QUIT)
					return;
    }
}


//------------------------------------------------------------  ExperimentTimer()
//
int ExperimentFSA(void){

	long int now,time_inc;
	Uint16 press;
	Uint8 change=0,i=0;
	float vx=0, vy=0, IDtmp;
	static Uint8 counter = 0;
	static Uint8 bRepeat = FALSE;
	static Uint8 preOsc = 0, slopeOsc=0, postOsc=0;

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
		if (now - g_last_change < LAST_CHANGE_INTERVAL*1000)
			change = FALSE;
		g_last_change =  now;
	}

	//Automata de estados, bifurca el flujo del programa
	switch(g_trial_state)
	{
		case TRIAL_STATE_REC_ERROR:///////////////////////////////////////////
		case TRIAL_STATE_NOREC_ERROR:
			sleep(ERROR_SCREEN_TIME/1000);
			ExperimentSaveData();
			g_trial_state = TRIAL_STATE_INTER_TRIAL;
			g_TrialConfig[g_trial_type].replications += 1;
			g_errors = 0;
			//ExperimentGraphics();
			break;
		case TRIAL_STATE_INTER_TRIAL:///////////////////////////////////
			if (!bRepeat) {
				if (!ExperimentSelect()) return FALSE;
				//ExperimentGraphics();
				bRepeat = TRUE;
				g_init_time = now;
				g_errors = 0;
				g_data_count = 0;
				g_last_change = 0;
				preOsc = 0;
				slopeOsc = 0;
				postOsc = 0;
			}
			else if (time_inc >= INTER_TRIAL_TIME*1000)	{
				g_trial_state = TRIAL_STATE_PRECONTACT;
				bRepeat = FALSE;
				//ExperimentGraphics();
			}
			break;
		case TRIAL_STATE_PRECONTACT:////////////////////////////////////
			if (press > MIN_PRESSURE){
				if (IsInTarget(x,y)) {
					g_trial_state = TRIAL_STATE_PREOSCILLATION;
					g_side = (g_side == 'R') ? 'L' : 'R';
					g_last_change = now;
				}
			}
			//ExperimentGraphics();
			break;
		case TRIAL_STATE_PREOSCILLATION:////////////////////////////////
			if (press > MIN_PRESSURE) {
				if (fabs(vx) > 0) { // Check for non null speed as trial init
					g_trial_state = TRIAL_STATE_OSCILLATION_PRESLOPE;
					g_init_time = now;
					//ExperimentGraphics();
				}
			}
			else {
				g_trial_state = TRIAL_STATE_REC_ERROR;
				//ExperimentGraphics();
			}
			break;
		case TRIAL_STATE_OSCILLATION_PRESLOPE://////////////////////////
			if (press > MIN_PRESSURE) {
				if (change){
					if (IsInTarget(x,y)){
						preOsc++;
						g_side = (g_side == 'R') ? 'L' : 'R';
					}
					else{
						g_errors++;
						g_side = (g_side == 'R') ? 'L' : 'R';
					}
				}
				if (preOsc >= OSCILLATIONS_BEFORE) {
					g_trial_state = TRIAL_STATE_OSCILLATION_SLOPE;
					g_slope_time = now;
					//ExperimentGraphics();
				}
			}
			else {
				g_trial_state = TRIAL_STATE_REC_ERROR;
				//ExperimentGraphics();
			}
			break;
		case TRIAL_STATE_OSCILLATION_SLOPE://///////////////////////////
			if (press > MIN_PRESSURE) {
				if (now - g_slope_time >= SLOPE_TIME*1000){
				//if (preOsc >= OSCILLATIONS_BEFORE + OSCILLATIONS_DURING) {
					g_trial_state = TRIAL_STATE_OSCILLATION_POSTSLOPE;
					g_ID = g_config.IDf;
					//ExperimentGraphics();
				}
				else {
					if (change){
						if (IsInTarget(x,y)){
							slopeOsc++;
							g_side = (g_side == 'R') ? 'L' : 'R';
							//g_ID = (g_config.IDf-g_config.ID0)/OSCILLATIONS_DURING * (preOsc-OSCILLATIONS_BEFORE)+ g_config.ID0;
						}
						else {
							g_errors++;
							g_side = (g_side == 'R') ? 'L' : 'R';
							//printf("Numero de errores = %d\n",g_errors);
							//fflush(stdout);
						}
					}
					IDtmp = (float)(g_config.IDf-g_config.ID0)/(float)SLOPE_TIME * ((float)(now - g_slope_time) / 1000.0) + g_config.ID0;
					if (fabs(g_ID - IDtmp) > MIN_ID_STEP) {
						g_ID = IDtmp;
						//ExperimentGraphics();
					}
				}
			}
			else {
				g_trial_state = TRIAL_STATE_REC_ERROR;
				//ExperimentGraphics();
			}
			break;
		case TRIAL_STATE_OSCILLATION_POSTSLOPE://///////////////////////
			if (press > MIN_PRESSURE){
				if (change){
					if (IsInTarget(x,y)){
						postOsc++;
						g_side = (g_side == 'R') ? 'L' : 'R';
					}
					else{
						g_errors++;
						g_side = (g_side == 'R') ? 'L' : 'R';
						//printf("Numero de errores = %d",g_errors);
						//fflush(stdout);
					}
				}
				if (postOsc >=  OSCILLATIONS_AFTER) {
					if (g_errors < ((float)(preOsc + slopeOsc + postOsc) * ERROR_PERCENT) ) {
					//if (TRUE){
						ExperimentSaveData();
						g_trial_state = TRIAL_STATE_INTER_TRIAL;
						//ExperimentGraphics();
					}
					else {
						printf("Too much errors: %d\n", g_errors);
						fflush(stdout);
						ExperimentSaveData();
						g_trial_state = TRIAL_STATE_REC_ERROR;
						//ExperimentGraphics();
					}
				}
			}
			else {
				g_trial_state = TRIAL_STATE_REC_ERROR;
				//ExperimentGraphics();
			}
			break;
		default:////////////////////////////////////////////////////////
			ExperimentEnd();
			break;
	} //end_switch

	// Guardamos estado detallado del movimiento si procede
	if (g_trial_state > TRIAL_STATE_PREOSCILLATION){
		g_TrialData[g_data_count].x = x;
		g_TrialData[g_data_count].y = y;
		g_TrialData[g_data_count].minx = minx();
		g_TrialData[g_data_count].maxx = maxx();
		g_TrialData[g_data_count].vx = vx;
		g_TrialData[g_data_count].vy = vy;
		g_TrialData[g_data_count].time = time_inc;
		g_TrialData[g_data_count].press = press;
		g_TrialData[g_data_count].oscillations = preOsc+slopeOsc+postOsc;
		g_TrialData[g_data_count].ID = g_ID;
		g_TrialData[g_data_count].side = g_side;
		g_TrialData[g_data_count].errors = g_errors;
		g_data_count++;
	}
	if (g_errors > ERROR_MAX)
		g_trial_state = TRIAL_STATE_REC_ERROR;
	return TRUE;
}	//end_function

//------------------------------------------------------------  ExperimentGraphics()
//
static void ExperimentGraphics(){
	pthread_t thread_id;
	pthread_attr_t tattr;
	static GRAPHARG args;

	args.ID = g_ID;
	args.side = g_side;
	args.state = g_trial_state;
	args.last_change = getmicroseconds() - g_last_change;
	args.x = x;
	args.y = y;

	pthread_attr_init(&tattr);
	pthread_attr_setdetachstate(&tattr,PTHREAD_CREATE_DETACHED);

	if ( pthread_create(&thread_id,&tattr,_ExperimentGraphics,(void*)&args) ) {
		perror("Error in graphics thread, closing program...\n");
		ExperimentGraphics();
	}
}
//------------------------------------------------------------  _ExperimentGraphics()
//
static void* _ExperimentGraphics(void* args) {
	static Uint32 bColor=0;
	float ID = 		((GRAPHARG *) args)->ID;
	char side = 	((GRAPHARG *) args)->side;
	Uint8 state = 	((GRAPHARG *) args)->state;
	float l_x = 	((GRAPHARG *) args)->x;
	float l_y = 	((GRAPHARG *) args)->y;
	long int last_change = ((GRAPHARG *) args)->last_change;

	switch(state) {
		case TRIAL_STATE_INTER_TRIAL:///////////////////////////////////////////
			ShowBackground(BG_INTER_COLOR);
			//ShowMessage();
			break;

		case TRIAL_STATE_PRECONTACT:///////////////////////////////////////////
			ShowBackground(BG_NORMAL_COLOR);
			//if (bColor % 100 == 0){
				if (bColor % 100 < 50){
					if (side == 'L')
						ShowTargets(TARGET_2ND_COLOR, TARGET_COLOR,ID);
					else
						ShowTargets(TARGET_COLOR, TARGET_2ND_COLOR,ID);
				}
				else {
					ShowTargets(TARGET_COLOR, TARGET_COLOR, ID);
				}
			//}
			ShowHandFeedback(l_x,l_y,FEEDBACK_COLOR);
			bColor++;
			break;
		case TRIAL_STATE_PREOSCILLATION:////////////////////////////////////
		case TRIAL_STATE_OSCILLATION_PRESLOPE:
		case TRIAL_STATE_OSCILLATION_POSTSLOPE:
			ShowBackground(BG_NORMAL_COLOR);
			ShowTargets(TARGET_COLOR, TARGET_COLOR, ID);
			ShowHandFeedback(l_x,l_y,FEEDBACK_COLOR);
			break;
		case TRIAL_STATE_OSCILLATION_SLOPE://///////////////////////////////
			if (last_change < GRAPHICS_DELAY*1000)
				side = (side == 'R') ? 'L' : 'R';
			ShowHalfBackground(BG_NORMAL_COLOR, side);
			//ShowBackground(BG_NORMAL_COLOR);
			ShowTarget(TARGET_COLOR, ID, side);
			ShowHandFeedback(l_x,l_y,FEEDBACK_COLOR);
			break;

		case TRIAL_STATE_REC_ERROR://///////////////////////////////////////
			ShowBackground(BG_ERROR_COLOR);
			//ShowMessage(g_error_msg);
			break;

		case TRIAL_STATE_NOREC_ERROR:///////////////////////////////////////
			ShowBackground(BG_ERROR_COLOR);
			//ShowMessage(g_error_msg);
			break;

		default:////////////////////////////////////////////////////////////
			break;
	}
	UpdateScreen();
	pthread_exit((void*)0);
}


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
		default:
			printf("default\n");
			break;
		fflush(stdout);
	}
}
