

CXX = gcc
CXXFLAGS = -g -Wall -O3
#CXXFLAGS = -Wall -O1 `sdl-config --cflags --libs` -lSDL_gfx

LIBS = -L/usr/lib -L. -lSDL -lSDL_gfx -lwacom -lpthread
LIBS1 = -L. -lGL -lGLU -lglut -lpthread -lwacom
LIBS2 = -L. -lpthread -lwacom -lSDL_gfx
INC = -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
MODULES = sdl.c draw.c fsa.c
PROGS = fitts
all: $(PROGS)

fitts:
	@ echo
	@ echo "	#  COMPILANDO EXPERIMENTO            #"
	@ echo
	$(CXX) $(CXXFLAGS) $(INC) main.c  $(MODULES) $(LIBS)   -o fitts



clean:
	rm -f *.o fitts fitts.log





