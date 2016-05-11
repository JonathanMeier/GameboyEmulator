#include <bitset>
#include <iostream>

using namespace std;

#define	CART	 0x100
#define CLKSPEED 4194304
#define DIVREG	 0xFF04 //if a game tries to write here, reset to 0
#define TIMA	 0xFF05
#define TMA		 0xFF06 //can be 4096, 262144, 65536, and 16384 Hz
#define TMC		 0xFF07
#define INTERSERV 0xFF0F //location of interrupts to service
#define INTEREN	 0xFFFF //location of interrupts allowed
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

typedef unsigned char byte;
typedef char sbyte;
typedef unsigned short bytebyte;
typedef bool bit;

class Memory {
public:
	Memory();
	~Memory();

	void write16(bytebyte address, byte value); //write to RAM array

	void setTimers(int cycles); //updates timers
	
	void setInterrupts(byte bit); //sets interrupts to be serviced if enabled
	bool testBit(byte a, byte b); //checks if bit in byte a at position byte b is enabled
	byte setBit(byte a, byte b); //returns byte a with bit at position byte b enabled
	void resetBit(byte &a, byte b); //sets bit at position byte b to 0 in byte a
	
	byte read16(bytebyte address); //read from RAM array

	byte RAM[0x10000]; //gameboy's memory
	bool binterrupt; //enabled/disabled if interrupts are/aren't allowed
private:
	int divCount = 0; //for the division register
	int clocksRemaining; //clocks remaining for timer to increment
	void DMAtransfer(byte value);
	void setClocksRemaining(); //set number of clock cycles needed for timing activity based on value at location 0xFF07
	void setDivReg(int cycles); //increments divider register.  Based on timing as well
};