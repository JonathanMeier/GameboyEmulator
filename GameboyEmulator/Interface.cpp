#include "Interface.h"

Interface::Interface() {}

Interface::Interface(Memory *m) {
	mem = m;

	//mem->RAM[LCDControl] = 0x91;
	//mem->RAM[LCDStatus] = 0x85;
	//mem->RAM[SCROLLY] = 0x00;
	//mem->RAM[SCROLLX] = 0x00;
	////mem->RAM[LY] = 0x90;
	//mem->RAM[LYC] = 0x00;
	//mem->RAM[BGP] = 0xFC;
	//mem->RAM[OBP0] = 0xFF;
	//mem->RAM[OBP1] = 0xFF;
	//mem->RAM[WY] = 0x00;
	//mem->RAM[WX] = 0x00;
	//UI

	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow("Gameboy Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, sizeX, sizeY, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	
	SDL_RenderSetLogicalSize(renderer, 160, 144);
	//SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
	//SDL_Delay(1000);
}

Interface::~Interface() {};

void Interface::setGraphics(int cycles) {
	setLCDStatus();
	if (mem->testBit(mem->read16(LCDControl), 7)) SLCounter -= cycles;
	else return;
	if (SLCounter <= 0) {
		mem->RAM[LY]++;
		byte curscanline = mem->read16(LY);
		SLCounter = 456;
		if (curscanline == 144) {
			mem->setInterrupts(0);
		}
		else if (curscanline > 153) {
			mem->RAM[LY] = 0;
		}
		else if (curscanline < 144) {
			//graphics stuff here
		}
	}
}

void Interface::setLCDStatus() {
	byte status = mem->read16(LCDStatus);
	if (mem->testBit(mem->read16(LCDControl), 7) == false) //if LCD is disabled
	{
		// set the mode to 1 during lcd disabled and reset scanline
		SLCounter = 456;
		mem->RAM[LY] = 0;
		status &= 252;
		status = mem->setBit(status, 0);
		mem->write16(LCDStatus, status);
		return;
	}
	byte currentline = mem->read16(LY);
	byte currentmode = status & 0x3;
	byte mode = 0;
	bool reqInt = false;
	// in vblank so set mode to 1 
	if (currentline >= 144)
	{
		mode = 1;
		status = mem->setBit(status, 0);
		mem->resetBit(status, 1);
		reqInt = mem->testBit(status, 4);
	}
	else
	{
		int mode2bounds = 456 - 80;
		int mode3bounds = mode2bounds - 172;
		// mode 2
		if (SLCounter >= mode2bounds)
		{
			mode = 2;
			status = mem->setBit(status, 1);
			mem->resetBit(status, 0);
			reqInt = mem->testBit(status, 5);
		}
		// mode 3
		else if (SLCounter >= mode3bounds)
		{
			mode = 3;
			status = mem->setBit(status, 1);
			status = mem->setBit(status, 0);
		}
		// mode 0
		else
		{
			mode = 0;
			mem->resetBit(status, 1);
			mem->resetBit(status, 0);
			reqInt = mem->testBit(status, 3);
		}
	}
	// just entered a new mode so request interupt
	if (reqInt && (mode != currentmode))
		mem->setInterrupts(1);
	// check the conincidence flag
	if (mem->read16(LY) == mem->read16(LYC))
	{
		status = mem->setBit(status, 2);
		if (mem->testBit(status, 6)) mem->setInterrupts(1);
	}
	else
	{
		mem->resetBit(status, 2);
	}
	mem->write16(LCDStatus, status);
}