#include "CPU.h"
#include <string>
#include <string.h>
#include <fstream>
#include <chrono> //sleep_for
#include <thread> //need for nanoseconds

using namespace std;

class ROM {
public:
	ROM() {};
	ROM(Z80 *c, Memory *m, Interface *r);
	~ROM();
	void loadROM(); //loads ROM into RAM (very simple without bank switching)
	byte readByte(); //read byte at PC
	void callInstr(); //OpCode map
	
	void runGame(); 
	
	//TEST FUNCTIONS
	byte readByteTest();
private:
	Z80 *gb;
	Memory *mem;
	Interface *graphics;
	unsigned long instrCount = 0; //count number of instructions have executed
	unsigned int getReg(unsigned int n); //returns register to pass to instructions
	void prefixCBTable(); //2nd opcode table
	byte *memCart;
};