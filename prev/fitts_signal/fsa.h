//////////////////////////////////////////////////////////////////
//  fsa.H
//
//
// Copyleft (L)		Marzo 2010		Jorge Ibáñez
//
//////////////////////////////////////////////////////////////////


int ExperimentFSA();

int GraphicsFSA(Uint8 state, float ID, char side);

int printFSA();

int ExperimentSelect();

void ExperimentSaveData();

///////////////////////////////////////////////////////////

Uint8 IsInTarget(float x, float y);

Uint8 UpdateSpeedBuffers(float* vx, float* vy, long int now);

long int getmicroseconds();

void itoa(int value, char* str, int base);

void strreverse(char* begin, char* end);
