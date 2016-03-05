LIBPATH = lib/SDL/
LIBLUA = lib/Lua
LIBS = -lSDL2 -lSDL2_mixer -lSDL2main -lSDL2_ttf -lSDL2_net -lSDL2_image -ldl -llua -g3 -m3dnow -mmmx -msse  -mpopcnt
CXXINCS = -I"include/SDL2" -I"Platform" -I"Console"
RM = rm -f

all:
	g++ main.cpp Map.cpp Platform/Graphics.cpp Console/InputLoop.cpp Items.cpp -o Soldier1D.exe $(LIBS) $(CXXINCS) -std=c++11
clean:
	RM *.exe

rebuild: clean all
