#include "ROMLoad.h"

ROM::ROM(Z80 *c, Memory *m, Interface *r) {
	gb = c;
	mem = m;
	graphics = r;
}

ROM::~ROM() {

}

void ROM::runGame() {
	cout << "Running emulation..." << endl;

	//int instrCount = 0;
	int cyclelimit = 69905; // 4194304 (clock rate) / 60 = 69905.  Execute number of instructions within 60 hz
	int cycles = 0;

	while (cycles < cyclelimit) {
		instrCount++;
		printf("Instruction: %u\n", instrCount);
		callInstr(); //DECODE AND EXECUTE
		cycles += gb->t; //ADD CYCLES FOR SPEED LIMITING / TIMING
		mem->setTimers(gb->t); //ADJUST TIMERS
		gb->checkInterrupts(); //CHECK FOR AND SERVICE INTERRUPTS
		graphics->setGraphics(gb->t); //DO GRAPHICS RELATED THINGS
		gb->printState(); //Print to console, debugging
		gb->t = 0; //Put cycles for previous instruction to 0 just to be safe.
	}
	//After the loop, the screen would be updated
}

unsigned int ROM::getReg(unsigned int n) {
	/*-----------
	| BIT | R |
	-----------
	| 111 | A |
	-----------
	| 000 | B |
	-----------
	| 001 | C |
	-----------
	| 010 | D |
	-----------
	| 011 | E |
	-----------
	| 100 | H |
	-----------
	| 101 | L |
	-----------
	| 110 | HL|
	---------- -*/
	switch (n) {
	case (7) :
		return 'a';
	case (0) :
		return 'b';
	case (1) :
		return 'c';
	case (2) :
		return 'd';
	case (3) :
		return 'e';
	case (4) :
		return 'h';
	case (5) :
		return 'l';
	case (6) :
		return 'hl';
	}
	printf("Error: Could not identify register.\n");
	return -1;
}

void ROM::loadROM() {
	unsigned long romSize;
	//byte *memCart;
	ifstream::pos_type size;
	string fileName = "Tetris.gb";
	ifstream file(fileName.c_str(), ios::in | ios::binary | ios::ate);
	size = file.tellg();
	cout << size << endl;
	romSize = (unsigned long)size;
	//cout << romSize << endl;
	memCart = new byte[romSize];
	file.seekg(0, ios::beg);
	file.read((char *)memCart, (streamsize)size);
	file.close();
	memcpy(&mem->RAM[0], &memCart[0], size);
	/*for (unsigned long count = 0; count < romSize; count++) {
		mem->RAM[count] = memCart[count];
	}*/
	cout << "Rom Loaded..." << endl;
}

byte ROM::readByte() {
	gb->reg.pc++;
	printf("0x%X\n", mem->RAM[gb->reg.pc - 1]);
	return mem->RAM[gb->reg.pc - 1];
}

byte ROM::readByteTest() {
	return 0x01;
}

void ROM::callInstr() {
	byte read = readByte();
	sbyte simm; //signed immediate
	struct immediate { //struct allows me to combine two data types as if it were one
		union {
			struct {
				byte imm8hi; //8 bit immediate, 'hi' part of 16 bit combination
				byte imm8lo; //'lo' part of 16 bit combination
			};
			bytebyte imm16;
		};
	};
	immediate imm;


	if (read >= 0x40 && read < 0x80) { //Captures most ld opcodes
		unsigned int hi, lo; //3 bits indicating register
		hi = read & 0x38; //0x38 = 0011 1000 bits 3-5
		hi >>= 3;
		lo = read & 0x07; //0x07 = 0000 0111 bits 0-2
		hi = getReg(hi);
		lo = getReg(lo);
		if (read == 0x76){
			gb->HALT();
		}
		else if (hi == 'hl') {
			gb->LDnn_n(hi, lo, false);
		}
		
		else {
			gb->LDn(hi, lo, false);
		}
	}
	else if (read >= 0x80 && read < 0xC0) { //Captures most ALU-based opcodes
		byte hi, lo; //half and half this time
		int par; //only one parameter needed
		hi = read & 0xF0;
		lo = read & 0x0F;
		par = read & 0x07; //0x07 = 0000 0111 - bits 0-2 contain register information
		par = getReg(par);
		if (lo < 0x08) {
			if (hi == 0x80) {
				gb->ADDa(par, false);
			}
			else if (hi == 0x90) {
				gb->SUBa(par, false);
			}
			else if (hi == 0xA0) {
				gb->ANDa(par, false);
			}
			else if (hi == 0xB0) {
				gb->OR(par, false);
			}
		}
		else {
			if (hi == 0x80) {
				gb->ADCa(par, false);
			}
			else if (hi == 0x90) {
				gb->SBCa(par, false);
			}
			else if (hi == 0xA0) {
				gb->XOR(par, false);
			}
			else if (hi == 0xB0) {
				gb->CPa(par, false);
			}
		}
	}
	else {
		switch (read) {
			//http://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html
		case (0x00) :
			gb->NOP();
			break;

		case (0x01) :
			imm.imm8hi = readByte();
			imm.imm8lo = readByte();
			gb->LDnn_imm('bc', imm.imm16);
			break;

		case (0x02) :
			gb->LDnn_n('bc', 'a', false);
			break;

		case (0x03) :
			gb->INCnn('bc');
			break;

		case (0x04) :
			gb->INCn('b');
			break;

		case (0x05) :
			gb->DECn('b');
			break;

		case (0x06) :
			imm.imm8hi = readByte();
			gb->LDn('b', imm.imm8hi, true);
			break;

		case (0x07) : 
			gb->RLCA();
			break;

		case (0x08) : 
			imm.imm8hi = readByte();
			imm.imm8lo = readByte();
			gb->LDa16sp(imm.imm16);
			break;

		case (0x09) :
			gb->ADDhlnn('bc');
			break;

		case (0x0A) :
			gb->LDn('a', 'bc', false);
			break;

		case (0x0B) :
			gb->DECnn('bc');
			break;

		case (0x0C) :
			gb->INCn('c');
			break;

		case (0x0D) :
			gb->DECn('c');
			break;

		case (0x0E) :
			imm.imm8hi = readByte();
			gb->LDn('c', imm.imm8hi, true);
			break;

		case (0x0F) :
			gb->RRCA();
			break;

		case (0x10) :
			//STOP instruction.
			break;

		case (0x11) :
			imm.imm8hi = readByte();
			imm.imm8lo = readByte();
			gb->LDnn_imm('de', imm.imm16);
			break;

		case (0x12) :
			gb->LDnn_n('de', 'a', false);
			break;

		case (0x13) :
			gb->INCnn('de');
			break;

		case (0x14) :
			gb->INCn('d');
			break;

		case (0x15) :
			gb->DECn('d');
			break;

		case (0x16) :
			imm.imm8hi = readByte();
			gb->LDn('d', imm.imm8hi, true);
			break;

		case (0x17) :
			gb->RLA();
			break;

		case (0x18) :
			simm = readByte();
			gb->JR(simm);
			break;

		case (0x19) :
			gb->ADDhlnn('de');
			break;

		case (0x1A) :
			gb->LDn('a', 'de', false);
			break;

		case (0x1B) :
			gb->DECnn('de');
			break;

		case (0x1C) :
			gb->INCn('e');
			break;

		case (0x1D) :
			gb->DECn('e');
			break;

		case (0x1E) :
			imm.imm8hi = readByte();
			gb->LDn('e', imm.imm8hi, true);
			break;

		case (0x1F) :
			gb->RRA();
			break;

		case (0x20) : //0 = NZ
			simm = readByte();
			gb->JRcc(0, simm);
			break;

		case (0x21) :
			imm.imm8hi = readByte();
			imm.imm8lo = readByte();
			gb->LDnn_imm('hl', imm.imm16);
			break;

		case (0x22) :
			gb->LDnn_n('hl', 'a', false, 1);
			break;

		case (0x23) :
			gb->INCnn('hl');
			break;

		case (0x24) :
			gb->INCn('h');
			break;

		case (0x25) :
			gb->DECn('h');
			break;

		case (0x26) :
			imm.imm8hi = readByte();
			gb->LDn('h', imm.imm8hi, true);
			break;

		case (0x27) :
			//DAA is fairly complex, not a lot of games use it.
			break;

		case (0x28) : //2 = Z
			simm = readByte();
			gb->JRcc(2, simm);
			break;

		case (0x29) :
			gb->ADDhlnn('hl');
			break;

		case (0x2A) :
			gb->LDn('a', 'hl', false, 1);
			break;

		case (0x2B) :
			gb->DECnn('hl');
			break;

		case (0x2C) :
			gb->INCn('l');
			break;

		case (0x2D) :
			gb->DECn('l');
			break;

		case (0x2E) :
			imm.imm8hi = readByte();
			gb->LDn('l', imm.imm8hi, true);
			break;

		case (0x2F) :
			gb->CPL();
			break;

		case (0x30) : //1 = NC
			simm = readByte();
			gb->JRcc(1, simm);
			break;

		case (0x31) :
			imm.imm8hi = readByte();
			imm.imm8lo = readByte();
			gb->LDnn_imm('sp', imm.imm16);
			break;

		case (0x32) :
			gb->LDnn_n('hl', 'a', false, -1);
			break;

		case (0x33) :
			gb->INCnn('sp');
			break;

		case (0x34) :
			gb->INChl();
			break;

		case (0x35) :
			gb->DECn('hl');
			break;

		case (0x36) :
			imm.imm8hi = readByte();
			gb->LDnn_n('hl', imm.imm8hi, true);
			break;

		case (0x37) :
			gb->SCF();
			break;

		case (0x38) : //3 = C
			simm = readByte();
			gb->JRcc(3, simm);
			break;

		case (0x39) :
			gb->ADDhlnn('sp');
			break;

		case (0x3A) :
			gb->LDn('a', 'hl', false, -1);
			break;

		case (0x3B) :
			gb->DECnn('sp');
			break;

		case (0x3C) :
			gb->INCn('a');
			break;

		case (0x3D) :
			gb->DECn('a');
			break;

		case (0x3E) :
			imm.imm8hi = readByte();
			gb->LDn('a', imm.imm8hi, true);
			break;

		case (0x3F) :
			gb->CCF();
			break;

		case (0xC0) :
			gb->RETcc(0); //0 represents NZ
			break;

		case (0xC1) :
			gb->POP('bc');
			break;

		case (0xC2) : 
			imm.imm8hi = readByte();
			imm.imm8lo = readByte();
			gb->JPcc(0, imm.imm16); //0 represents NZ
			break;

		case (0xC3) : 
			imm.imm8hi = readByte();
			imm.imm8lo = readByte();
			gb->JP(imm.imm16);
			break;

		case (0xC4) :
			imm.imm8hi = readByte();
			imm.imm8lo = readByte();
			gb->CALLcc(0, imm.imm16); //0 represents NZ
			break;

		case (0xC5) :
			gb->PUSH('bc');
			break;

		case (0xC6) :
			imm.imm8hi = readByte();
			gb->ADDa(imm.imm8hi, true);
			break;

		case (0xC7) :
			gb->RST(0);
			break;

		case (0xC8) :
			gb->RETcc(2);
			break;

		case (0xC9) :
			gb->RET();
			break;

		case (0xCA) :
			imm.imm8hi = readByte();
			imm.imm8lo = readByte();
			gb->JPcc(2, imm.imm16);
			break;

		case (0xCB) : //go to prefixCBTable
			gb->sr('m', 1);
			gb->sr('t', 4);
			prefixCBTable();
			break;

		case (0xCC) :
			imm.imm8hi = readByte();
			imm.imm8lo = readByte();
			gb->CALLcc(2, imm.imm16);
			break;

		case (0xCD) :
			imm.imm8hi = readByte();
			imm.imm8lo = readByte();
			gb->CALL(imm.imm16);
			break;

		case (0xCE) :
			imm.imm8hi = readByte();
			gb->ADCa(imm.imm8hi, true);
			break;

		case (0xCF) :
			gb->RST(0x08);
			break;

		case (0xD0) :
			gb->RETcc(1);
			break;

		case (0xD1) :
			gb->POP('de');
			break;

		case (0xD2) :
			imm.imm8hi = readByte();
			imm.imm8lo = readByte();
			gb->JPcc(1, imm.imm16);
			break;

		case (0xD4) :
			imm.imm8hi = readByte();
			imm.imm8lo = readByte();
			gb->CALLcc(1, imm.imm16);
			break;

		case (0xD5) :
			gb->PUSH('de');
			break;

		case (0xD6) :
			imm.imm8hi = readByte();
			gb->SUBa(imm.imm8hi, true);
			break;

		case (0xD7) :
			gb->RST(0x10);
			break;

		case (0xD8) :
			gb->RETcc(3);
			break;

		case (0xD9) :
			gb->RETI();
			break;

		case (0xDA) :
			imm.imm8hi = readByte();
			imm.imm8lo = readByte();
			gb->JPcc(3, imm.imm16);
			break;

		case (0xDC) :
			imm.imm8hi = readByte();
			imm.imm8lo = readByte();
			gb->CALLcc(3, imm.imm16);
			break;

		case (0xDE) :
			imm.imm8hi = readByte();
			gb->SBCa(imm.imm8hi, true);
			break;

		case (0xDF) :
			gb->RST(0x18);
			break;

		case (0xE0) :
			imm.imm8hi = readByte();
			gb->LDH(0, imm.imm8hi);
			break;

		case (0xE1) :
			gb->POP('hl');
			break;

		case (0xE2) :
			gb->LDc(0);
			break;

		case (0xE5) :
			gb->PUSH('hl');
			break;

		case (0xE6) :
			imm.imm8hi = readByte();
			gb->ANDa(imm.imm8hi, true);
			break;

		case (0xE7) :
			gb->RST(0x20);
			break;

		case (0xE8) :
			simm = readByte();
			gb->ADDsp_i(simm);
			break;

		case (0xE9) : 
			gb->JPhl();
			break;

		case (0xEA) :
			imm.imm8hi = readByte();
			imm.imm8lo = readByte();
			gb->LDa16(false, imm.imm16);
			break;

		case (0xEE) :
			imm.imm8hi = readByte();
			gb->XOR(imm.imm8hi, true);
			break;

		case (0xEF) :
			gb->RST(0x28);
			break;

		case (0xF0) :
			imm.imm8hi = readByte();
			gb->LDH(1, imm.imm8hi);
			break;

		case (0xF1) :
			gb->POP('af');
			break;

		case (0xF2) :
			gb->LDc(1);
			break;

		case (0xF3) :
			gb->DI();
			break;

		case (0xF5) :
			gb->PUSH('af');
			break;

		case (0xF6) :
			imm.imm8hi = readByte();
			gb->OR(imm.imm8hi, true);
			break;

		case (0xF7) :
			gb->RST(0x30);
			break;

		case (0xF8) :
			simm = readByte();
			gb->LDhlsp8(simm);
			break;

		case (0xF9) :
			gb->LDn1_n2('sp', 'hl');
			break;

		case (0xFA) : 
			imm.imm8hi = readByte();
			imm.imm8lo = readByte();
			gb->LDa16(1, imm.imm16);
			break;

		case (0xFB) :
			gb->EI();
			break;

		case (0xFE) :
			imm.imm8hi = readByte();
			gb->CPa(imm.imm8hi, true);
			break;

		case (0xFF) :
			gb->RST(0x38);
			break;

		}
	}
}

void ROM::prefixCBTable() {
	byte read = readByte();
	if (read <= 0x08) {
		gb->RLCn(getReg(read % 8));
	}
	else if (read > 0x08 && read <= 0x0f) {
		gb->RRCn(getReg(read % 8));
	}
	else if (read > 0x0f && read <= 0x17) {
		gb->RLn(getReg(read % 8));
	}
	else if (read > 0x17 && read <= 0x1f) {
		gb->RRn(getReg(read % 8));
	}
	else if (read > 0x1f && read <= 0x27) {
		gb->SLAn(getReg(read % 8));
	}
	else if (read > 0x27 && read <= 0x2f) {
		gb->SRAn(getReg(read % 8));
	}
	else if (read > 0x2f && read <= 0x37) {
		gb->SWAPn(getReg(read % 8));
	}
	else if (read > 0x37 && read <= 0x3f) {
		gb->SRLn(getReg(read % 8));
	}
	else if (read > 0x3f && read <= 0x7f) {
		gb->BITn((getReg(read % 8)), (read / 8) - 8);
	}
	else if (read > 0x7f && read <= 0xbf) {
		gb->RESn((getReg(read % 8)), (read / 8) - 8);
	}
	else if (read > 0xbf) {
		gb->SETn((getReg(read % 8)), (read / 8) - 8);
	}
}