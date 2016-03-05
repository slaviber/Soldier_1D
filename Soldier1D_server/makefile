LIBPATH = lib/SDL/
LIBLUA = lib/Lua
LIBS = -lSDL2 -lSDL2_mixer -lSDL2main -lSDL2_ttf -lSDL2_net -lSDL2_image -g3 -m3dnow -mmmx -msse -llua -lpthread -mpopcnt
CXXINCS = -I"include/SDL2" -I"Platform" -I"Console" -I"include/Lua53"
RM = rm -f

all:
	g++ Socket.cpp main.cpp -o Soldier1D.exe $(LIBS) $(CXXINCS) -std=c++11
clean:
	RM *.exe

rebuild: clean all
