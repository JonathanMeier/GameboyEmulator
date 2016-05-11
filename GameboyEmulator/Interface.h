#include "SDL.h"
#include "Memory.h"

using namespace std;

#define LCDControl	0xFF40
#define LCDStatus   0xFF41
#define SCROLLY		0xFF42
#define SCROLLX		0xFF43
#define LY			0xFF44
#define LYC			0xFF45
#define WY			0xFF4A
#define WX			0xFF4B
#define BGP			0xFF47
#define OBP0		0xFF48
#define OBP1		0xFF49

#define FPS			60

class Interface {
public:
	Interface();
	Interface(Memory *m);
	~Interface();

	void setGraphics(int cycles); //handles graphics related things

	SDL_Event e; //events for SDL
	
	//parameters for SDL.  Pretty much useless.
	int posX = 1000; 
	int posY = 1000;
	int sizeX = 800;
	int sizeY = 800;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *tiles[192];

private:
	Memory *mem;
	int SLCounter = 456; //scaneline counter.  Needed for graphics timing
	void setLCDStatus(); //Code taken and modified from http://www.codeslinger.co.uk/pages/projects/gameboy/lcd.html
};