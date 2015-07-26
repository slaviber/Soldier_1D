LIBPATH = lib/SDL/
LIBS = -L"$(LIBPATH)" $(LIBPATH)SDL2.dll -lSDL2_mixer -lmingw32 -lSDL2main $(LIBPATH)libfreetype-6.dll -lSDL2_ttf -lSDL2_net -lSDL2_image -lgdi32 -lwinmm -g3 -m3dnow -mmmx -msse -lWs2_32 -lwsock32 -lIphlpapi -mpopcnt
CXXINCS = -I"include/SDL2" -I"Platform" -I"Console"
RM = rm -f

all:
	g++ main.cpp Map.cpp Platform/Graphics.cpp Console/InputLoop.cpp -o Soldier1D.exe $(LIBS) $(CXXINCS) -std=c++11 -m32
clean:
	RM *.exe

rebuild: clean all