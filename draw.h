//////////////////////////////////////////////////////////////////
//  draw.h
//
//
// Copyleft (L)		Marzo 2010		Jorge Ibáñez
//
//////////////////////////////////////////////////////////////////




void ShowTargets(Uint32 colorL, Uint32 colorR, float ID);

void ShowTarget(Uint32 color, float ID, char side);

void ShowBackground(Uint32 color);

void ShowHalfBackground(Uint32 color, char side);

void UpdateScreen();

void ShowMessage();

void ShowHandFeedback(float x,float y, Uint32 color);

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
