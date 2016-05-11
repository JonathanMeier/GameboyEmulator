#include "CPU.h"
#include <iostream>
#include <fstream>
#include <bitset>

//using namespace std;

Z80::Z80(Memory *memory) {
	//Appropriate initial values
	reg.af = 0x01B0;
	reg.bc = 0x0013;
	reg.de = 0x00D8;
	reg.hl = 0x014D;
//----------------------------------------------------
	//0x80 = zero	
	//0x40 = last op was subtraction	
	//0x20 = half-carry (lower nybble carry)	
	//0x10 = carry
//----------------------------------------------------
	reg.sp = 0xFFFE;
	reg.pc = 0x0100;

	m = 0;
	t = 0;

	mem = memory;
}

Z80::~Z80() {};

void Z80::printState() {
	//printf(" A: %X \n F: %X \n B: %X \n C: %X \n D: %X \n E: %X \n HL: %X \n SP: %X \n PC: %X \n Flags: 0x%X \n", reg.a, reg.f, reg.b, reg.c, reg.d, reg.e, reg.hl, reg.sp, reg.pc, reg.f);
	printf(" AF: %X \n BC: %X \n DE: %X \n HL: %X \n SP: %X \n PC: %X \n", reg.af, reg.bc, reg.de, reg.hl, reg.sp, reg.pc);
	printf(" LCDC: %X \n Stat: %X \n LY: %X \n IE: %X \n IF: %X \n", mem->RAM[LCDControl], mem->RAM[LCDStatus], mem->RAM[LY], mem->RAM[INTEREN], mem->RAM[INTERSERV]);
}

//template <typename type>
const int Z80::gr(int r) { //get register
	switch (r) {
	case 'a':
		return reg.a;
	case 'b':
		return reg.b;
	case 'c':
		return reg.c;
	case 'd':
		return reg.d;
	case 'e':
		return reg.e;
	case 'f':
		return reg.f;
	case 'h':
		return reg.h;
	case 'l':
		return reg.l;
	case 'af':
		return reg.af;
	case 'bc' :
		return reg.bc;
	case 'de' :
		return reg.de;
	case 'hl' :
		return reg.hl;
	case 'sp':
		return reg.sp;
	case 'pc':
		return reg.pc;
	case 'm':
		return m;
	case 't':
		return t;
	};
	return 1;
}

void Z80::sr(int r, int val) { //set register
	switch (r) {
	case 'a':
		reg.a = val;
		break;
	case 'b':
		reg.b = val;
		break;
	case 'c':
		reg.c = val;
		break;
	case 'd':
		reg.d = val;
		break;
	case 'e':
		reg.e = val;
		break;
	case 'f':
		reg.f = val;
		break;
	case 'h':
		reg.h = val;
		break;
	case 'l':
		reg.l = val;
		break;
	case 'af':
		reg.af = val;
		break;
	case 'bc' :
		reg.bc = val;
		break;
	case 'de' :
		reg.de = val;
		break;
	case 'hl' :
		reg.hl = val;
		break;
	case 'pc' :
		reg.pc = val;
		break;
	case 'sp':
		reg.sp = val;
		break;
	case 'm':
		m = val;
	case 't':
		t = val;
	};
}

void Z80::setFlagZ(bool f) { //0x80
	if (f) { //set flag
		sr('f', (gr('f') | 0x80));
	}
	else { //reset flag
		sr('f', gr('f') - (gr('f') & 0x80));
	}
}

void Z80::setFlagC(bool f) { //0x10
	if (f) { //set flag
		sr('f', (gr('f')|0x10));
	}
	else { //reset flag
		sr('f', gr('f') - (gr('f') & 0x10));
	}
}

void Z80::setFlagH(bool f) { //0x20
	if (f) { //set flag
		sr('f', (gr('f') | 0x20));
	}
	else { //reset flag
		sr('f', gr('f') - (gr('f') & 0x20));
	}
}

void Z80::setFlagS(bool f) { //0x40
	if (f) { //set flag
		sr('f', (gr('f') | 0x40));
	}
	else { //reset flag
		sr('f', gr('f') - (gr('f') & 0x40));
	}
}

//return flag status
const byte Z80::getFlagZ() { 
	return gr('f') & 0x80;
}

const byte Z80::getFlagC() {
	return gr('f') & 0x10;
}

const byte Z80::getFlagH() {
	return gr('f') & 0x20;
}

const byte Z80::getFlagS() {
	return gr('f') & 0x40;
}

bool Z80::testHalfCarryAdd(byte a, byte b) {
	return ((((a & 0x0f) + (b & 0x0f)) & 0x10) == 0x10);
}

bool Z80::testHalfCarryAdd16(bytebyte a, bytebyte b) {
	return (((a & 0x7ff) + (b & 0x7ff)) > 0x7ff);
}

bool Z80::testHalfCarrySub(byte a, byte b) {
	return ((a & 0x0f) < (b & 0x0f));
}

bool Z80::testCarryAdd(byte a, byte b) {
	return ((a + b) > 0xff);
}

bool Z80::testCarryAdc(byte a, byte b) {
	return ((a + b + getFlagC()) > 0xff);
}

bool Z80::testCarryAdd16(bytebyte a, bytebyte b) {
	return ((a + b) > 0xffff);
}

bool Z80::testCarrySub(byte a, byte b) {
	return (a < b);
}

bool Z80::testCarrySbc(byte a, byte b) {
	return (a < (b + getFlagC()));
}

//INTERRUPT FUNCTIONS----------------------------------------------------

void Z80::checkInterrupts() {
	if (mem->binterrupt) {
		byte toService = mem->read16(INTERSERV);
		byte canService = mem->read16(INTEREN);
		if (toService != 0) {
			for (int i = 0; i < 5; i++) {
				if (mem->testBit(toService, i)) {
					if (mem->testBit(canService, i)) serviceInterrupts(i);
				}
			}
		}
	}
}

void Z80::serviceInterrupts(int inter) {
	mem->binterrupt = false;
	byte toService = mem->read16(INTERSERV);
	mem->resetBit(toService, inter);
	mem->write16(INTERSERV, toService);

	reg.sp--;
	mem->write16(reg.sp, reg.pc_p);
	reg.sp--;
	mem->write16(reg.sp, reg.pc_c);

	switch (inter) {
	case (0):
		reg.pc = 0x40;
		break;
	case (1):
		reg.pc = 0x48;
		break;
	case (2):
		reg.pc = 0x50;
		break;
	case (4):
		reg.pc = 0x60;
		break;
	}
}

//-------------------------------------------------------------------------

void Z80::makeBit(byte &b) {
	switch (b) {
	case(0) :
		b = 1;
		break;
	case(1) :
		b = 2;
		break;
	case(2) :
		b = 4;
		break;
	case(3) :
		b = 8;
		break;
	case(4) :
		b = 16;
		break;
	case(5) :
		b = 32;
		break;
	case(6) :
		b = 64;
		break;
	case(7) :
		b = 128;
		break;
	}
}

template <typename type>
type Z80::getValue(type r) {
	return gr(r);
}

//Instruction set

void Z80::ADCa(int n, bool imm) { //A <- A + reg/imm + carry flag
	byte data;
	if (imm) {
		data = n;
		sr('t', 8);
		sr('m', 2);
	}
	else if (n == 'hl') {
		data = mem->read16(gr(n));
		sr('t', 8);
		sr('m', 1);
	}
	else {
		data = gr(n);
		sr('t', 4);
		sr('m', 1);
	}
	setFlagS(false); //reset sub flag
	setFlagC(testCarryAdc(gr('a'), data));
		//setFlagC((gr('a') + data + getFlagC()) > 0xff);
		//setFlagC((((gr('a') & 0xff) + (data & 0xff) + (getFlagC() & 0xff)) & 0x100) == 0x100); //set carry flag
	setFlagH(testHalfCarryAdd(gr('a'), data));
		//setFlagH(((gr('a') & 0xf) + (data & 0xf) + (getFlagC() & 0xf)) > 0xf); //set half carry flag
	sr('a', (gr('a') + data + getFlagC()));
	setFlagZ(gr('a') == 0); //set zero flag
}

void Z80::ADDa(int n, bool imm) { //My instruction names include the instruction type
	//sr = set register             and register 
	//gr = get register
	byte data; //Stores unsigned char based on the type of ADD needed
	if (imm) {
		data = n; //If n is an immediate
		sr('t', 8); //Number of clock cycles for the instruction
		sr('m', 2); //Number of bytes needed by the instruction.
	}
	else if (n == 'hl') {
		data = mem->read16(gr(n));  //If n = 'hl', then I need to get 
		sr('t', 8);					//the value in memory at the location 
		sr('m', 1);					//stored in hl. 
	}
	else {
		data = gr(n); 
		sr('t', 4);
		sr('m', 1);
	}
	setFlagS(false); //reset sub flag
	setFlagC(testCarryAdd(gr('a'), data)); //Set Carry flag
	setFlagH(testHalfCarryAdd(gr('a'), data)); //Set Half Carry Flag
	sr('a', (gr('a') + data));
	setFlagZ(gr('a') == 0); //set zero flag
}

void Z80::ADDhlnn(int nn) {
	bytebyte data = gr(nn);
	setFlagS(false); //reset sub flag
	setFlagC(testCarryAdd16(gr('hl'), data));
		//setFlagC((gr('hl') + n) > 0xffff); //set carry flag
	setFlagH(testHalfCarryAdd16(gr('hl'), data));
		//setFlagH(((gr('hl') & 0x7ff) + (data & 0x7ff)) > 0x7ff); //set half carry flag if carry from 11th bit
	sr('hl', (gr('hl') + data));
	sr('m', 1); //adjust timers
	sr('t', 8);
}

void Z80::ADDsp_i(char imm) {
	setFlagS(false); //reset sub flag
	setFlagZ(false); //reset zero flag
		//setFlagC((gr('sp') + imm) > 0xffff); //set carry flag
	setFlagC(testCarryAdd16(gr('sp'), imm));
		//setFlagH(((gr('sp') & 0x7ff) + (imm & 0x7ff)) > 0x7ff); //Since it's signed, this might be wrong?  I think the flags are sign agnostic though.
	setFlagH(testHalfCarryAdd16(gr('sp'), imm));
		//setFlagC((((gr('sp') & 0xffff) + (imm & 0xffff)) & 0x10000) == 0x10000); //set carry flag
		//setFlagH((((gr('sp') & 0x7ff) + (imm & 0x7ff)) & 0x1000) == 0x1000); //set half carry flag (not half for this one)
	sr('sp', (gr('sp') + imm));
	sr('m', 2); //adjust timers
	sr('t', 16);
}

void Z80::ANDa(int n, bool imm) {
	byte data;
	if (imm) {
		data = n;
		sr('t', 8);
		sr('m', 2);
	}
	else if (n == 'hl') {
		data = mem->read16(gr(n));
		sr('t', 8);
		sr('m', 1);
	}
	else {
		data = gr(n);
		sr('t', 4);
		sr('m', 1);
	}
	sr('a', gr('a') & data);
	setFlagZ(gr('a') == 0);
	setFlagS(false); //reset sub flag
	setFlagH(true); //unconditionally set half carry flag
	setFlagC(false); //reset carry flag
}

void Z80::BITn(int n, byte b) {
	byte data;
	makeBit(b);
	if (n == 'hl') {
		data = mem->read16(gr(n));
		sr('t', 16);
	}
	else {
		data = gr(n);
		sr('t', 8);
	}
	setFlagZ(!(data & b));
	setFlagS(false); //reset sub flag
	setFlagH(true); //unconditionally set half carry flag
	sr('m', 2);
	//carry flag ignored
}

void Z80::CALL(bytebyte adr) { //Put PC on stack and jump to adr
	mem->write16(reg.sp - 1, reg.pc_p);
	mem->write16(reg.sp - 2, reg.pc_c);
	reg.sp -= 2;
	reg.pc = adr;
}

void Z80::CALLcc(byte cc, bytebyte adr) {
	//cc = 0 -> NZ
	//cc = 1 -> NC
	//cc = 2 -> Z
	//cc = 3 -> C
	bool test;
	switch (cc) {
	case (0) :
		test = (getFlagZ() != 0x80);
		break;
	case (1) :
		test = (getFlagC() != 0x10);
		break;
	case (2) :
		test = (getFlagZ() == 0x80);
		break;
	case (3) :
		test = (getFlagC() == 0x10);
		break;
	}
	if (test) CALL(adr);
	else {
		sr('t', 12);
		sr('m', 3);
	}

}

void Z80::CCF() { //Complement Carry Flag
	setFlagC(getFlagC() != 0x10);
	setFlagS(false); 
	setFlagH(false);
	sr('m', 1);
	sr('t', 4);
}

void Z80::CPa(int n, bool imm) {
	byte data;
	if (imm) {
		data = n;
		sr('t', 8);
		sr('m', 2);
	}
	else if (n == 'hl') {
		data = mem->read16(gr(n));
		sr('t', 8);
		sr('m', 1);
	}
	else {
		data = gr(n);
		sr('t', 4);
		sr('m', 1);
	}
	setFlagZ(reg.a == data);
	setFlagS(true);
		//setFlagH(!(((gr('a') & 0xf0) - data) < 0x10));
	setFlagH(testHalfCarrySub(gr('a'), data));
		//setFlagC(reg.a > data); 
	setFlagC(testCarrySub(gr('a'), data));
}

void Z80::CPL() {
	sr('a', ~gr('a')); //flip bits
	setFlagS(true); //set sub flag
	setFlagH(true); //set half carry flag
	sr('m', 1);
	sr('t', 4);
}

void Z80::DAA() {

	//THE FOLLOWING DETAILS COME FROM http://www.emutalk.net/threads/41525-Game-Boy/page108

	/*Detailed info DAA
		Instruction Format :
	OPCODE                    CYCLES
		--------------------------------
		27h                       4


		Description :
		This instruction conditionally adjusts the accumulator for BCD addition
		and subtraction operations.For addition(ADD, ADC, INC) or subtraction
		(SUB, SBC, DEC, NEC), the following table indicates the operation performed :

	--------------------------------------------------------------------------------
		| | C Flag | HEX value in | H Flag | HEX value in | Number | C flag |
		| Operation | Before | upper digit | Before | lower digit | added | After |
		| | DAA | (bit 7 - 4) | DAA | (bit 3 - 0) | to byte | DAA |
		| ------------------------------------------------------------------------------ |
		| | 0 | 0 - 9 | 0 | 0 - 9 | 00 | 0 |
		| ADD | 0 | 0 - 8 | 0 | A - F | 06 | 0 |
		| | 0 | 0 - 9 | 1 | 0 - 3 | 06 | 0 |
		| ADC | 0 | A - F | 0 | 0 - 9 | 60 | 1 |
		| | 0 | 9 - F | 0 | A - F | 66 | 1 |
		| INC | 0 | A - F | 1 | 0 - 3 | 66 | 1 |
		| | 1 | 0 - 2 | 0 | 0 - 9 | 60 | 1 |
		| | 1 | 0 - 2 | 0 | A - F | 66 | 1 |
		| | 1 | 0 - 3 | 1 | 0 - 3 | 66 | 1 |
		| ------------------------------------------------------------------------------ |
		| SUB | 0 | 0 - 9 | 0 | 0 - 9 | 00 | 0 |
		| SBC | 0 | 0 - 8 | 1 | 6 - F | FA | 0 |
		| DEC | 1 | 7 - F | 0 | 0 - 9 | A0 | 1 |
		| NEG | 1 | 6 - F | 1 | 6 - F | 9A | 1 |
		| ------------------------------------------------------------------------------ |


		Flags:
	C:   See instruction.
		N : Unaffected.
		P / V : Set if Acc.is even parity after operation, reset otherwise.
		H : See instruction.
		Z : Set if Acc.is Zero after operation, reset otherwise.
		S : Set if most significant bit of Acc.is 1 after operation, reset otherwise.

		Example :

		If an addition operation is performed between 15 (BCD) and 27 (BCD), simple decimal
		arithmetic gives this result :

		15
		+ 27
		----
		42

		But when the binary representations are added in the Accumulator according to
		standard binary arithmetic :

		 0001 0101  15
			 + 0010 0111  27
			 -------------- -
			 0011 1100  3C

			 The sum is ambiguous.The DAA instruction adjusts this result so that correct
			 BCD representation is obtained :

		 0011 1100  3C result
			 + 0000 0110  06 + error
			 -------------- -
			 0100 0010  42 Correct BCD! 
	*/
}

void Z80::DECn(int n) {
	byte data;
	if (n == 'hl') {
		data = mem->read16(gr(n));
		sr('t', 12);
		sr('m', 1);
	}
	else {
		data = gr(n);
		sr('t', 4);
		sr('m', 1);
	}
	setFlagH(testHalfCarrySub(data, 1));
	setFlagS(true); //set sub flag
	data--;
	setFlagZ(data == 0); //set zero flag
	if (n == 'hl') mem->write16(gr(n), data);
	else sr(n, data);
}

void Z80::DECnn(int nn) {
	sr(nn, (gr(nn) - 1));
	sr('m', 1);
	sr('t', 8);
	//no flags affected
}

void Z80::DI() {
	mem->binterrupt = false;
	sr('t', 4);
	sr('m', 1);
}

void Z80::EI() {
	mem->binterrupt = true;
	sr('t', 4);
	sr('m', 1);
}

void Z80::INCn(int n) {
	sr(n, (gr(n) + 1));
	setFlagS(false); //reset sub flag
	setFlagZ(gr(n) == 0); //set zero flag
	setFlagH(testHalfCarryAdd(gr(n), 1));
	//setFlagH(((gr('a') & 0xf) + 1) > 0xf); //set half carry flag
	sr('m', 1);
	sr('t', 4);
}

void Z80::INCnn(int nn) {
	sr(nn, (gr(nn) + 1));
	sr('m', 1);
	sr('t', 8);
}

void Z80::INChl() {
	reg.hl++;
	setFlagS(false); //reset sub flag
	setFlagZ(reg.hl == 0); //set zero flag
	setFlagH(testHalfCarryAdd16(reg.hl, 1));
	sr('m', 1);
	sr('t', 12);
}

void Z80::JP(bytebyte adr) {
	reg.pc = adr;
	sr('m', 3);
	sr('t', 16);
}

void Z80::JPcc(byte cc, bytebyte adr) {
	//cc = 0 -> NZ (zero flag = false)
	//cc = 1 -> NC (carry flag = false)
	//cc = 2 -> Z
	//cc = 3 -> C
	bool b = false;
	switch (cc) {
	case (0):
		b = (getFlagZ() == 0);
		break;
	case (1):
		b = (getFlagC() == 0);
		break;
	case (2):
		b = (!(getFlagZ() == 0));
		break;
	case (3):
		b = (!(getFlagC() == 0));
		break;
	}
	if (b) JP(adr);
	else sr('t', 12);
}

void Z80::JPhl() {
	reg.pc = mem->read16(gr('hl'));
	sr('t', 4);
	sr('m', 1);
}

void Z80::JR(char n) { //char cause unsigned requirement
	reg.pc += n;
	sr('t', 12);
	sr('m', 2);
}

void Z80::JRcc(byte cc, sbyte n) {
	//cc = 0 -> NZ
	//cc = 1 -> NC
	//cc = 2 -> Z
	//cc = 3 -> C
	sr('m', 2);
	sr('t', 8);
	switch (cc) {
	case (0):
		if (getFlagZ() == 0) {
			JR(n);
		}
		else sr('t', 8);
		break;
	case (1):
		if (getFlagC() == 0) {
			JR(n);
		}
		else sr('t', 8);
		break;
	case (2):
		if (!(getFlagZ() == 0)) {
			JR(n);
		}
		else sr('t', 8);
		break;
	case (3):
		if (!(getFlagC() == 0)) {
			JR(n);
		}
		else sr('t', 8);
		break;
	}
}

void Z80::HALT() {

}

void Z80::LDn(int n1, int n2, bool imm, int offset) {
	byte data;
	if (imm) {
		data = n2;
		sr('t', 8);
		sr('m', 2);
	}
	else if (n2 > 127) {
		data = mem->read16(gr(n2));
		sr(n2, gr(n2) + offset);
		sr('t', 8);
		sr('m', 1);
	}
	else {
		data = gr(n2);
		sr('t', 4);
		sr('m', 1);
	}
	sr(n1, data);
}

void Z80::LDnn_n(int n1, int n2, bool imm, int offset) { //Write to memory
	byte data;
	if (imm) {
		data = n2;
		sr('t', 12);
		sr('m', 2);
	}
	else {
		data = gr(n2);
		sr('t', 8);
		sr('m', 1);
	}
	mem->write16(gr(n1), data); //Write to address held by register "n1"
	sr(n1, gr(n1) + offset); //Some instructions increment or decrement n1
}

void Z80::LDnn_imm(int nn, bytebyte imm) {
	sr(nn, imm);
	sr('t', 12);
	sr('m', 3);
}

void Z80::LDhlsp8(sbyte imm) {
	setFlagZ(false); //reset
	setFlagS(false); //reset
	//setFlagC(((gr('hl') & 0xffff) + (imm & 0xffff) + (gr('sp') & 0xffff)) > 0xffff); //set carry flag
	setFlagC(testCarryAdd16(gr('sp'), imm));
	//setFlagH(((gr('hl') & 0x7ff) + (imm & 0x7ff) + (gr('sp') & 0x7ff)) > 0x7ff); //set half carry flag (not half for this one)
	setFlagH(testHalfCarryAdd16(gr('sp'), imm));
	reg.hl = reg.sp + imm;
	sr('m', 2);
	sr('t', 12);
}

void Z80::LDn1_n2(int n1, int n2) {
	sr(n1, gr(n2));
	sr('m', 1);
	sr('t', 8);
}

void Z80::LDa16(bool test, bytebyte imm) {
	//test = 0: (a16) <- A
	//test = 1: A <- (a16)
	if (test == true) sr('a', mem->read16(imm));
	else {
		mem->write16(imm, gr('a'));
	}
	sr('t', 16);
	sr('t', 3);
}

void Z80::LDa16sp(bytebyte addr) {
	mem->write16(addr, gr('sp'));
	sr('m', 3);
	sr('t', 20);
}

void Z80::LDH(bool rw, byte addr) {
	//Since memory addressing is 16-bit and registers are 8 bit,
	//there is an instruction for using one register instead
	//of a register pair to access memory.

	//rw = 0: A into addr
	//rw = 1: addr into A
	if (rw == 0) {
		mem->write16(0xFF00 + addr, gr('a')); //gr is get register.
	}					
	else {									  
		sr('a', mem->read16(0xFF00 + addr));  //sr is set register.
	}
	sr('m', 2);
	sr('t', 12);
}

void Z80::LDc(bool rw) {
	//rw = 0: A into (C)
	//rw = 1: (C) into A
	if (rw == 0) {
		mem->write16(mem->read16(0xFF00 + gr('c')), gr('a'));
	}
	else {
		sr('a', mem->read16(0xFF00 + gr('c')));
	}
	sr('t', 8);
	sr('m', 2);
}

void Z80::NOP() {
	sr('m', 1);
	sr('t', 4);
}

void Z80::OR(int n, bool imm) {
	byte data;
	if (imm) {
		data = n;
		sr('t', 8);
		sr('m', 2);
	}
	else if (n == 'hl') {
		data = mem->read16(gr(n));
		sr('t', 8);
		sr('m', 1);
	}
	else {
		data = gr(n);
		sr('t', 4);
		sr('m', 1);
	}
	sr('a', gr('a') | data);
	setFlagZ(gr('a') == 0);
	setFlagS(false); //reset sub flag
	setFlagH(false); //unconditionally set half carry flag
	setFlagC(false); //reset carry flag
}

void Z80::POP(int n) {
	union data{
		struct {
			byte lo;
			byte hi;
		};
		bytebyte both;
	};
	data d;
	d.lo = mem->read16(gr('sp'));
	d.hi = mem->read16(gr('sp') + 1);
	sr(n, d.both);
	sr('sp', gr('sp') + 2);
	sr('t', 12);
	sr('m', 1);
}

void Z80::PUSH(int n) {
	bytebyte data = gr(n);
	mem->write16((gr('sp') - 1), ((data & 0xff00) >> 8));
	mem->write16((gr('sp') - 2), (data & 0x00ff));
	sr('sp', gr('sp') - 2);
	sr('t', 16);
	sr('m', 1);
}

void Z80::RESn(int n, byte b) {
	byte data;
	makeBit(b);
	if (n = 'hl') {
		data = mem->read16(gr(n));
		sr('t', 16);
	}
	else {
		data = gr(n);
		sr('t', 8);
	}
	b ^= 0xff; //flip bits so that position to be reset is 0
	data &= b; //and them so that position to be reset always fails
	//flags unaffected
	if (n == 'hl') mem->write16(gr(n), data);
	else sr(n, data);
	sr('m', 2);
}

void Z80::REShl(byte b) {

}

void Z80::RET() {
	reg.pc_c = mem->read16(reg.sp);
	reg.pc_p = mem->read16(reg.sp + 1);
	reg.sp += 2;
	sr('t', 16);
	sr('m', 1);
}

void Z80::RETcc(byte cc) {
	//cc = 0 -> NZ
	//cc = 1 -> NC
	//cc = 2 -> Z
	//cc = 3 -> C
	bool b = false;
	switch (cc) {
	case (0) :
		b = (getFlagZ() == 0);
		break;
	case (1) :
		b = (getFlagC() == 0);
		break;
	case (2) :
		b = (!(getFlagZ() == 0));
		break;
	case (3) :
		b = (!(getFlagC() == 0));
		break;
	}
	if (b) {
		reg.pc_c = mem->read16(reg.sp);
		reg.sp++;
		reg.pc_p = mem->read16(reg.sp);
		reg.sp++;
		sr('t', 20);
	}
	else {
		sr('t', 8);
	}
	sr('m', 1);
}

void Z80::RETI() {
	RET();
	mem->binterrupt = true;
}

void Z80::RLA() {
	byte temp1 = reg.a & 0x80; //last bit of register
	byte temp2 = getFlagC(); //carry flag bit
	setFlagC(temp1 == 0x80);
	reg.a <<= 1;
	reg.a |= (temp2 >> 4); //carry flag is 5th bit in register, so shift 4 to front
	setFlagZ(false);
	setFlagS(false);
	setFlagH(false);
	sr('t', 4);
	sr('m', 1);
}

void Z80::RLn(int n) {
	byte data;
	if (n == 'hl') {
		data = mem->read16(gr(n));
		sr('t', 16);
	}
	else {
		data = gr(n);
		sr('t', 8);
	}
	byte temp1 = data & 0x80; //last bit of register
	byte temp2 = getFlagC(); //carry flag bit
	setFlagC(temp1 == 0x80);
	data <<= 1;
	data |= (temp2 >> 4); //carry flag is 5th bit in register, so shift 4 to front
	setFlagZ(data == 0);
	setFlagS(false);
	setFlagH(false);
	if (n == 'hl') mem->write16(gr(n), data);
	else sr(n, data);
	sr('m', 2);
}

void Z80::RLCA() {
	byte temp = reg.a & 0x80;
	setFlagC(temp == 0x80);
	reg.a <<= 1;
	reg.a |= (temp >> 7);
	setFlagZ(false);
	setFlagS(false);
	setFlagH(false);
	sr('t', 4);
	sr('m', 1);
}

void Z80::RLCn(int n) {
	byte data;
	if (n == 'hl') {
		data = mem->read16(gr(n));
		sr('t', 16);
	}
	else {
		data = gr(n);
		sr('t', 8);
	}
	byte temp = data & 0x80;
	setFlagC(temp == 0x80);
	data <<= 1;
	data |= (temp >> 7);
	setFlagZ(data == 0);
	setFlagS(false);
	setFlagH(false);
	if (n == 'hl') mem->write16(gr(n), data);
	else sr(n, data);
	sr('m', 2);
}

void Z80::RRn(int n) {
	byte data;
	if (n == 'hl') {
		data = mem->read16(gr(n));
		sr('t', 16);
	}
	else {
		data = gr(n);
		sr('t', 8);
	}
	byte temp1 = data & 1; //last bit of register
	byte temp2 = getFlagC(); //carry flag bit
	setFlagC(temp1 == 1);
	data >>= 1;
	data |= (temp2 << 3); //carry flag is 5th bit in register, so shift 3 to back
	setFlagZ(data == 0);
	setFlagS(false);
	setFlagH(false);
	if (n == 'hl') mem->write16(gr(n), data);
	else sr(n, data);
	sr('m', 2);
}

void Z80::RRCn(int n) { //Rotate Right, store right most bit in carry flag
	byte data;
	if (n == 'hl') {
		data = mem->read16(gr(n));
		sr('t', 16);
	}
	else {
		data = gr(n);
		sr('t', 8);
	}
	byte temp = data & 1;
	setFlagC(temp == 1);
	data >>= 1;
	data |= (temp << 7);
	setFlagZ(data == 0);
	setFlagS(false);
	setFlagH(false);
	if (n == 'hl') mem->write16(gr(n), data);
	else sr(n, data);
	sr('m', 2);
}

void Z80::RRCA() {
	byte temp = reg.a & 1;
	setFlagC(temp == 1);
	reg.a >>= 1;
	reg.a |= (temp << 7);
	setFlagZ(false);
	setFlagS(false);
	setFlagH(false);
	sr('m', 1);
	sr('t', 4);
}

void Z80::RRA() {
	byte temp1 = reg.a & 1; //last bit of register
	byte temp2 = getFlagC(); //carry flag bit
	setFlagC(temp1 == 1);
	reg.a >>= 1;
	reg.a |= (temp2 << 3); //carry flag is 5th bit in register, so shift 3 to back
	setFlagZ(false);
	setFlagS(false);
	setFlagH(false);
	sr('t', 4);
	sr('m', 1);
}

void Z80::RST(byte val) {
	reg.sp--;
	mem->write16(reg.sp, reg.pc_p);
	reg.sp--;
	mem->write16(reg.sp, reg.pc_c);
	reg.pc = val;
	sr('t', 16);
	sr('m', 1);
}

void Z80::SUBa(int n, bool imm) {
	byte data;
	if (imm) {
		data = n;
		sr('t', 8);
		sr('m', 2);
	}
	else if (n == 'hl') {
		data = mem->read16(gr(n));
		sr('t', 8);
		sr('m', 1);
	}
	else {
		data = gr(n);
		sr('t', 4);
		sr('m', 1);
	}
	//setFlagC(gr('a') < data); //Might be wrong
	setFlagC(testCarrySub(gr('a'), data));
	//setFlagH(!(((gr('a') & 0xf0) - data) < 0x10)); //Might be wrong
	setFlagH(testHalfCarrySub(gr('a'), data));
	setFlagS(true);
	sr('a', gr('a') - data);
	setFlagZ(gr('a') == 0);
}

void Z80::SBCa(int n, bool imm) {
	byte data;
	if (imm) {
		data = n;
		sr('t', 8);
		sr('m', 2);
	}
	else if (n == 'hl') {
		data = mem->read16(gr(n));
		sr('t', 8);
		sr('m', 1);
	}
	else {
		data = gr(n);
		sr('t', 4);
		sr('t', 1);
	}
	//setFlagC((gr('a') > (getFlagC() + data))); //Might be wrong
	setFlagC(testCarrySbc(gr('a'), data));
	//setFlagH(!(((gr('a') & 0xf0) - data - getFlagC()) < 0x10)); //Might be wrong
	setFlagH(testHalfCarrySub(gr('a'), data));
	setFlagS(true);
	sr('a', gr('a') - data - getFlagC());
	setFlagZ(gr('a') == 0);	
}

void Z80::SCF() {
	setFlagC(true);
	setFlagH(false);
	setFlagS(false);
	//zero flag unaffected
}

void Z80::SETn(int n, byte b) {
	byte data;
	makeBit(b);
	if (n = 'hl') {
		data = mem->read16(gr(n));
		sr('t', 16);
	}
	else {
		data = gr(n);
		sr('t', 8);
	}
	data |= b; 
	if (n == 'hl') mem->write16(gr(n), data);
	else sr(n, data);
	sr('m', 2);
}

void Z80::SLAn(int n) {
	byte data;
	if (gr(n) == 'hl') {
		data = mem->read16(gr(n));
		sr('t', 16);
	}
	else {
		data = gr(n);
		sr('t', 8);
	}
	setFlagC((data & 0x80) == 0x80);
	data <<= 1;
	setFlagZ(gr(n) == 0);
	setFlagS(false);
	setFlagH(false);
	if (n == 'hl') mem->write16(gr(n), data);
	else sr(n, data);
	sr('m', 2);
}

void Z80::SRAn(int n) {
	byte data;
	if (gr(n) == 'hl') {
		data = mem->read16(gr(n));
		sr('t', 16);
	}
	else {
		data = gr(n);
		sr('t', 8);
	}
	setFlagC((data & 1) == 1);
	byte temp = data & 0x80;
	data >>= 1;
	data |= temp;
	setFlagZ(gr(n) == 0);
	setFlagS(false);
	setFlagH(false);
	if (n == 'hl') mem->write16(gr(n), data);
	else sr(n, data);
	sr('m', 2);
}

void Z80::SRLn(int n) {
	byte data;
	if (gr(n) == 'hl') {
		data = mem->read16(gr(n));
		sr('t', 16);
	}
	else {
		data = gr(n);
		sr('t', 8);
	}
	setFlagC((data & 1) == 1);
	data >>= 1;
	setFlagZ(gr(n) == 0);
	setFlagS(false);
	setFlagH(false);
	if (n == 'hl') mem->write16(gr(n), data);
	else sr(n, data);
	sr('m', 2);
}

void Z80::SWAPn(int n) {
	byte data;
	if (n == 'hl') {
		data = mem->read16(gr(n));
		sr('t', 16);
	}
	else {
		data = gr(n);
		sr('t', 8);
	}
	data = ((data & 0x0f) << 4 | (data & 0xf0) >> 4); //split halves, shift, and OR them
	setFlagZ(data == 0);
	setFlagC(false);
	setFlagH(false);
	setFlagS(false);
	if (n == 'hl') mem->write16(gr(n), data);
	else sr(n, data);
	sr('m', 2);
}

void Z80::XOR(int n, bool imm) {
	byte data;
	if (imm) {
		data = n;
		sr('t', 8);
		sr('m', 2);
	}
	else if (n == 'hl') {
		data = mem->read16(gr(n));
		sr('t', 8);
		sr('m', 1);
	}
	else {
		data = gr(n);
		sr('t', 4);
		sr('m', 1);
	}
	sr('a', gr('a') ^ data);
	setFlagZ(gr('a') == 0);
	setFlagS(false); //reset sub flag
	setFlagH(false); //unconditionally set half carry flag
	setFlagC(false); //reset carry flag
}

//DEBUG FUNCTIONS
const char Z80::getFlag() {
	return gr('f');
}
