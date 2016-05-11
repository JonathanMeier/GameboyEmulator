#include <string>
#include <string.h>
#include <fstream>

using namespace std;

//The following code is heavily based off of an open source gameboy emulator, DMGBoy
//The source code has been modified to fit my design, but I do not claim it as my own

class ROM {
public:
	ROM(string fileName, string path);
	ROM(char *buffer, unsigned long size, string path);
	~ROM();

	char *getData();
	unsigned int getSize();
	string getName();
	bool loaded();
	char read(short direction);
	void write(short direction, char value);
	void print(int beg, int end);

	void saveMBC(ofstream *file); //Create save states?
	void loadMBC(ifstream *file); //Load save states?

private:
	unsigned long romSize;
	string name;
	bool load;
	char *memCart;

	char (*ptrRead)(short); //pointer to a function
	void (*ptrWrite)(short, char); //pointer to a function
	void checkCart(string path = "");
	int checkRomSize(int headerSize, int fileSize);
};