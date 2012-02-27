//////////////////////////////////////////////////////////////////
//  main.h
//
//
// Copyleft (L)		Marzo 2010		Jorge Ibáñez
//
//////////////////////////////////////////////////////////////////


#define APP_NAME	"SDL Fitts Experiment"
#define APP_VERSION	"0.0.0.0.0"


typedef enum
{
	WACOM_SCANMODE_CALLBACK,
	WACOM_SCANMODE_NOCALLBACK,
	WACOM_SCANMODE_MAX
} WACOM_SCANMODE;

typedef enum{
	SDL_VIDEO_MODE,
	SDL_OPENGL_VIDEO_MODE,
	OPENGL_VIDEO_MODE,
	VIDEO_MODE_MAX
} VIDEO_MODE;

//////////////////////////////////////////////////////////////////
//		PROTOTIPOS DE LAS FUNCIONES								//
//////////////////////////////////////////////////////////////////


//From main.c
static void ParseCmdLine(int argc, char** argv);

static void ExperimentSetup(int argc, char **argv);

static void ExperimentEventMask();

static void ExperimentLoop(void);

static void ExperimentEnd(void);

static void SetDelay(long int ns);

//From sdlgl.c
void OnExit();

void OnInit();


//From fsa.c
int ExperimentFSA();


// From draw.c
void InitGraphicsThread();

void EndGraphicsThread();

//From WacomLib
void WacomInit(int Mode, void (*WacomSignalHandler)(int));

void WacomClose(void);
