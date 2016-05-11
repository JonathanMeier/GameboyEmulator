#include "ROMLoad.h"

typedef unsigned char byte;

int main(int argc, char *args[]) { //TEST CODE								   
	Memory *mem;
	Interface *win;
	Z80 *proc;
	ROM *rom;
	mem = new Memory();
	win = new Interface(mem);
	proc = new Z80(mem);
	rom = new ROM(proc, mem, win);
	
	rom->loadROM();
	
	bool quit = false;

	SDL_Init(SDL_INIT_VIDEO);

	win->tiles[0] = SDL_CreateTexture(win->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 8, 8);
	Uint32 *pixels = new Uint32[8 * 8];
	memset(pixels, 255, 8 * 8 * sizeof(Uint32));
	while (!quit)
	{
		
		SDL_UpdateTexture(win->tiles[0], NULL, pixels, 8 * sizeof(Uint32));
		SDL_WaitEvent(&win->e);

		switch (win->e.type)
		{
		case SDL_QUIT:
			quit = true;
			break;
		}
		SDL_RenderClear(win->renderer);
		SDL_RenderCopy(win->renderer, win->tiles[0], NULL, NULL);
		SDL_RenderPresent(win->renderer);
		rom->runGame();
	}
	delete[] pixels;
	SDL_DestroyTexture(win->tiles[0]);
	SDL_DestroyRenderer(win->renderer);
	SDL_DestroyWindow(win->window);
	SDL_Quit();
	return 1;
}