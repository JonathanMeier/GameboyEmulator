#include "Memory.h"

Memory::Memory() {
	clocksRemaining = 1024;
	for (int i = 0; i < 10000; i++) {
		RAM[i] = 0;
	}
	RAM[0xFF05] = 0x00;
	RAM[0xFF06] = 0x00;
	RAM[0xFF07] = 0x00;
	RAM[0xFF10] = 0x80;
	RAM[0xFF11] = 0xBF;
	RAM[0xFF12] = 0xF3;
	RAM[0xFF14] = 0xBF;
	RAM[0xFF16] = 0x3F;
	RAM[0xFF17] = 0x00;
	RAM[0xFF19] = 0xBF;
	RAM[0xFF1A] = 0x7F;
	RAM[0xFF1B] = 0xFF;
	RAM[0xFF1C] = 0x9F;
	RAM[0xFF1E] = 0xBF;
	RAM[0xFF20] = 0xFF;
	RAM[0xFF21] = 0x00;
	RAM[0xFF22] = 0x00;
	RAM[0xFF23] = 0xBF;
	RAM[0xFF24] = 0x77;
	RAM[0xFF25] = 0xF3;
	RAM[0xFF26] = 0xF1;
	RAM[0xFF40] = 0x91;
	RAM[0xFF42] = 0x00;
	RAM[0xFF43] = 0x00;
	RAM[0xFF45] = 0x00;
	RAM[0xFF47] = 0xFC;
	RAM[0xFF48] = 0xFF;
	RAM[0xFF49] = 0xFF;
	RAM[0xFF4A] = 0x00;
	RAM[0xFF4B] = 0x00;
	RAM[0xFF0F] = 0xE1;
	RAM[0xFFFF] = 0x00;

}

Memory::~Memory() {

}

void Memory::write16(bytebyte address, byte value) {
	if (address <= 0x7FFF) {
		//read only, not worried about bank switching
	}
	else if (address >= 0xE000 && address < 0xFE00) {
		RAM[address] = value;
		RAM[address - 0x2000] = value;
	}
	else if (address >= 0xFEA0 && address < 0xFEFF) {
		//restricted area
	}
	else if (address == 0xFF46) {
		DMAtransfer(value);
	}
	else if (address == TMC) { //need to figure out if TMC is being changed to change clocksRemaining
		byte oldTMC = RAM[TMC] & 0x03; //Also it seems like changing to the same value doesn't change how long the timer needs to run.
		RAM[address] = value;
		if ((RAM[TMC] & 0x03) != oldTMC) setClocksRemaining();
	}
	else if (address == DIVREG) {
		RAM[address] = 0;
	}
	else if (address == LY) {
		RAM[address] = 0;
	}
	else {
		RAM[address] = value;
	}
}

void Memory::DMAtransfer(byte value) {
	bytebyte addr = value << 8; //since the source addr is data * 100, shift 8 instead.
	for (int i = 0; i < 0xA0; i++) {
		write16(0xFE00 + i, read16(addr + i));
	}
}

void Memory::setTimers(int cycles) {
	setDivReg(cycles);
	if (RAM[TMC] & 0x04 == 0x04) { //3rd bit of TMC determines if the timer is enabled or not
		clocksRemaining -= cycles;
		if (clocksRemaining <= 0) {
			setClocksRemaining();
			if (read16(TIMA) == 0xff) {
				write16(TIMA, read16(TMA));
				setInterrupts(2);
			}
			else {
				write16(TIMA, read16(TIMA) + 1);
			}
		}
	}
}

void Memory::setInterrupts(byte bit) {
	byte temp = setBit(read16(INTERSERV), bit);
	write16(INTERSERV, temp);
}

void Memory::setClocksRemaining() {
	byte clk = read16(TMC) & 0x03;
	switch (clk) {
	case (0):
		clocksRemaining = CLKSPEED / 4096;
		break;
	case (1):
		clocksRemaining = CLKSPEED / 262144;
		break;
	case (2):
		clocksRemaining = CLKSPEED / 65536;
		break;
	case (3):
		clocksRemaining = CLKSPEED / 16382;
		break;
	}
}

void Memory::setDivReg(int cycles) {
	divCount += cycles;
	if (divCount >= 255) {
		divCount = 0;
		RAM[DIVREG]++;
	}
}

bool Memory::testBit(byte a, byte b) {
	switch (b) {
	case (0):
		return (a & 1) == 1;
	case (1):
		return (a & 2) == 2;
	case (2):
		return (a & 4) == 4;
	case (3):
		return (a & 8) == 8;
	case (4):
		return (a & 16) == 16;
	case (5):
		return (a & 32) == 32;
	case (6):
		return (a & 64) == 32;
	case (7):
		return (a & 128) == 128;
	}
}

void Memory::resetBit(byte &a, byte b) {
	switch (b) {
	case (0):
		a &= (1 ^ 0xff);
		break;
	case (1):
		a &= (2 ^ 0xff);
		break;
	case (2):
		a &= (4 ^ 0xff);
		break;
	case (3):
		a &= (8 ^ 0xff);
		break;
	case (4):
		a &= (16 ^ 0xff);
		break;
	case (5):
		a &= (32 ^ 0xff);
		break;
	case (6):
		a &= (64 ^ 0xff);
		break;
	case (7):
		a &= (128 ^ 0xff);
		break;
	}
}

byte Memory::setBit(byte a, byte b) {
	switch (b) {
	case (0):
		return (a | 1);
	case (1):
		return (a | 2);
	case (2):
		return (a | 4);
	case (3):
		return (a | 8);
	case (4):
		return (a | 16);
	case (5):
		return (a | 32);
	case (6):
		return (a | 64);
	case (7):
		return (a | 128);
	}
}

byte Memory::read16(bytebyte address) {
	return RAM[address];
}