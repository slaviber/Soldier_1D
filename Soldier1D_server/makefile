LIBPATH = lib/SDL/
LIBLUA = lib/Lua
LIBS = -L"$(LIBLUA)" -L"$(LIBPATH)" $(LIBPATH)SDL2.dll -lSDL2_mixer -lmingw32 -lSDL2main $(LIBPATH)libfreetype-6.dll -lSDL2_ttf -lSDL2_net -lSDL2_image -lgdi32 -lwinmm -g3 -m3dnow -mmmx -msse -lWs2_32 -lwsock32 -lIphlpapi -llua -mpopcnt
CXXINCS = -I"include/SDL2" -I"Platform" -I"Console" -I"include/Lua53"
RM = rm -f

all:
	g++ Socket.cpp main.cpp -o Soldier1D.exe $(LIBS) $(CXXINCS) -std=c++11 -m32
clean:
	RM *.exe

rebuild: clean all