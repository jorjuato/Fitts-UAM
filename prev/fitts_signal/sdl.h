//////////////////////////////////////////////////////////////////
//  sdlgl_control.h
//	Funciones de inicializacion de la libreria SDL y del contexto
//	OpenGL para graficar en 3D.
//  Cabeceras con definiciones para el acceso a las librerias gráficas
//  SDL y OpenGL, así como definiciones de las caracteristicas físicas
//  del dispositivo gráfico donde se ejecuta el programa
//
//
// Copyleft (L)		Marzo 2010		Jorge Ibáñez
//
//////////////////////////////////////////////////////////////////




//------------------------------------------------------------------------------//
//																				//
//					Graphic Engines' Setup Routines	Prototypes					//
//																				//
//------------------------------------------------------------------------------//

///////////////////////////////////////////////////		API Callback funtions

void OnExit();

void OnInit(int mode);

void OnDraw();

void OnReshape(int w, int h);

SDL_Surface* LoadImage(char *file);

///////////////////////////////////////////////////		Basic Initializing and Drawing SDL/GL API

//static int setup_opengl();

//static SDL_Surface* setup_sdlgl();



typedef enum{
	SDL_VIDEO_MODE,
	SDL_OPENGL_VIDEO_MODE,
	OPENGL_VIDEO_MODE,
	VIDEO_MODE_MAX
} VIDEO_MODE;
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
