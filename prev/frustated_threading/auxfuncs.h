/************************************************************************##
** 	auxfuncs.h										 			 		 ##
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



void ParseCmdLine(int argc, char** argv);

void ExperimentSetup(int argc, char **argv);

int ExperimentSelect();

void ExperimentFiles();

void ExperimentEventMask();

void ExperimentSaveData();

Uint8 IsInTarget(float x, float y);

void SetDelay(long int ns);

long int getmicroseconds();

Uint8 UpdateSpeedBuffers(float* vx, float* vy, long int now);

void itoa(int value, char* str, int base);

void strreverse(char* begin, char* end);
