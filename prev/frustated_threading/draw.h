//////////////////////////////////////////////////////////////////
//  draw.h
//
//
// Copyleft (L)		Marzo 2010		Jorge Ibáñez
//
//////////////////////////////////////////////////////////////////



void ShowTargets(Uint32 colorL, Uint32 colorR, float ID);

void ShowBackground(Uint32 color);

void UpdateScreen();

void ShowMessage();

SDL_Surface* LoadImage(char *file);
