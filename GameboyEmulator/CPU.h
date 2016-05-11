#include "Interface.h"
#include <map>
#include <string>

using namespace std;

typedef unsigned char byte; //for 8-bit values
typedef unsigned short bytebyte; //for 16-bit values
typedef char sbyte; //for signed 8-bit values

class Z80 {
public:
	Z80() {};
	Z80(Memory *memory);
	~Z80();

	const int gr(int r); //getRegister
	void sr(int r, int val); //setRegister

	void setFlagZ(bool f); //set zero flag
	void setFlagC(bool f); //set carry flag
	void setFlagH(bool f); //set half-carry flag
	void setFlagS(bool f); //set subtraction flag

	const byte getFlagZ(); //get zero flag
	const byte getFlagC(); //get carry flag
	const byte getFlagH(); //get half-carry flag
	const byte getFlagS(); //get subtraction flag

	//tests called by instructions to set/reset flags
	bool testHalfCarryAdd(byte a, byte b);
	bool testHalfCarryAdd16(bytebyte a, bytebyte b);
	bool testHalfCarrySub(byte a, byte b);

	bool testCarryAdd(byte a, byte b);
	bool testCarryAdc(byte a, byte b); //a + b + carry flag
	bool testCarryAdd16(bytebyte a, bytebyte b);
	bool testCarrySub(byte a, byte b);
	bool testCarrySbc(byte a, byte b); //a - b - carry flag

//DEBUGGING----------------------------------------------------------
	void printState(); //Prints all registers
	const char getFlag();
	//-------------------------------------------------------------------

//INTERRUPT FUNCTIONS---------------------------------------------------

	void checkInterrupts();
	void serviceInterrupts(int inter);

//---------------------------------------------------------------------

	void makeBit(byte &b); //Sets b to the value enabling bit in position b

	template <typename type>
	type getValue(type r); //determine if r is a register and return value

//INSTRUCTIONS----------------------------------------------------------
		// An 'a' or 'hl' in the name means the result is stored in reg a or hl
		// 'n' generally means it goes with an 8-bit register
		// 'nn' generally means it goes with a 16-bit register combo
		// 'adr' means address
		// 'imm/i' means immediate
		// 'cc' means call condition.
	void ADCa(int n, bool imm); //A <- A + reg + carry flag
	void ADDa(int n, bool imm);
	void ADDhlnn(int nn);
	void ADDsp_i(char imm);
	void ANDa(int n, bool imm);
	void BITn (int n, byte b);
	void CALL(bytebyte adr); //NEEDS MEMORY INTERFACE TO IMPLEMENT
	void CALLcc(byte cc, bytebyte adr); //NEEDS MEMORY INTERFACE TO IMPLEMENT
	void CCF();
	void CPa(int n, bool imm);
	void CPL();
	void DAA(); //IMPLEMENT LATER, VERY HARD
	void DECn(int n);
	void DECnn(int nn);
	void DI(); //IMPLEMENT LATER, NO CLUE (disable interrupts)
	void EI(); //IMPLEMENT LATER, NO CLUE (enable interrupts)
	void INCn(int n);
	void INCnn(int nn);
	void INChl(); //special increment for hl, can't find much information on it, guessing it works like ADDhl
	void JP(bytebyte adr); //NEED MEMORY TO IMPLEMENT
	void JPcc(byte cc, bytebyte adr); //NEED MEMORY TO IMPLEMENT
	void JPhl(); //NEED MEMORY TO IMPLEMENT
	void JR(char imm); //NEED MEMORY TO IMPLEMENT
	void JRcc(byte cc, sbyte imm); //NEED MEMORY TO IMPLEMENT
	void HALT(); //IMPLEMENT LATER, NO CLUE

	//LOADS-------------------------------------------------------------------
	void LDn(int n1, int n2, bool imm, int offset = 0);
	void LDnn_n(int n1, int n2, bool imm, int offset = 0);
	void LDnn_imm(int nn, bytebyte imm);
	void LDa16sp(bytebyte addr);
	void LDH(bool rw, byte addr); //rw = read or write with address: 0 = A into addr, 1 = addr into A
	void LDc(bool rw);
	void LDhlsp8(sbyte imm); //hl = sp + r8
	void LDn1_n2(int n1, int n2); //used for both n into nn and nn into n
	void LDa16(bool test, bytebyte imm);
	//-------------------------------------------------------------------------

	void NOP();
	void OR(int n, bool imm);
	void POP(int n);
	void PUSH(int n);
	void RESn(int n, byte b);
	void REShl(byte b);
	void RET();
	void RETcc(byte cc);
	void RETI(); //Need interrupts implemented

	void RLn(int n);
	void RLA();
	void RLCA();
	void RLCn(int n);
	void RRn(int n);
	void RRCn(int n);
	void RRCA(); 
	void RRA(); 
	void RST(byte val);
	void SETn(int n, byte b);
	void SLAn(int n); 
	void SRAn(int n); 
	void SWAPn(int n); //swaps first and last nibbles of n, need to think of how to implement
	void SRLn(int n); 

	void SUBa(int n, bool imm);
	void SBCa(int n, bool imm);
	void SCF();
	
	//void STOP();  Halt's CPU.  Not sure what I'd do here, hasn't been an issue
	
	void XOR(int n, bool imm);

//END INSTRUCTIONS---------------------------------------------------------------------------

	struct registers {
		struct {
			union {
				struct {
					byte f;
					byte a;
				};
				bytebyte af;
			};
		};

		struct {
			union {
				struct {
					byte c;
					byte b;
				};
				bytebyte bc;
			};
		};

		struct {
			union {
				struct {
					byte e;
					byte d;
				};
				bytebyte de;
			};
		};

		struct {
			union {
				struct {
					byte l;
					byte h;
				};
				bytebyte hl;
			};
		};
		struct {
			union {
				struct { //split for RET and RST instructions
					byte pc_c;
					byte pc_p;
				};
				bytebyte pc;
			};
		};

		bytebyte sp;
		//bytebyte pc;
	};
	registers reg;
	bytebyte m, t; //clock

private:
	Memory *mem;
};

//BINARY BITWISE OPERATORS
// & -> AND
// ^ -> XOR
// | -> OR
// ~ -> NOT