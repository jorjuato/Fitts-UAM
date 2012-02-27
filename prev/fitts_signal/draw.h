//////////////////////////////////////////////////////////////////
//  draw.h
//
//
// Copyleft (L)		Marzo 2010		Jorge Ibáñez
//
//////////////////////////////////////////////////////////////////


void InitGraphicsThread();

void EndGraphicsThread();

void ExperimentGraphics();

void* GraphicsThread();

int GraphicsFSA(Uint8 state, float ID, char side);

void ShowTargets(Uint32 colorL, Uint32 colorR, float ID);

void ShowTarget(Uint32 color, float ID, char side);

void ShowBackground(Uint32 color);

void ShowHalfBackground(Uint32 color, char side);

void UpdateScreen();

void ShowMessage();
