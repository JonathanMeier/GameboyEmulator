#include "ROMLoad.h"
#include <iostream>

ROM::ROM(string fileName, string path) { //make sure you figure out what this does exactly
										 //try to write this as your own you lazy idiot
	memCart = NULL;
	ifstream::pos_type size;
	ifstream file(fileName.c_str(), ios::in | ios::binary | ios::ate);
	if (file.is_open()) {
		size = file.tellg(); //position of character
		this->romSize = (unsigned long)size;
		memCart = new char[size];
		file.seekg(0, ios::beg); //sets position of next character
		file.read((char *)memCart, (streamsize)size);
		cout << memCart << endl;
		file.close();

		cout << fileName << ":\nFile loaded in memory correctly" << endl;

		checkCart(path);

		load = true;
	}
	else {
		cerr << fileName << ": Error trying to open the file" << endl; //something about errors, figure out how this works
		load = false;
	}
}

ROM::ROM(char *buffer, unsigned long size, string path) {
	romSize = size;
	memCart = buffer;
	checkCart(path);
	load = true;
}

ROM::~ROM() {

};

char *ROM::getData() {
	char *a = 0;
	return a;
}

unsigned int ROM::getSize() {
	return 1;
}

string ROM::getName() {
	return "a";
}

bool ROM::loaded() {
	return 1;
}

char ROM::read(short direction) {
	return 1;
}

void ROM::write(short direction, char value) {
}

void ROM::print(int beg, int end) {

}

void ROM::saveMBC(ofstream *file) {

}

void ROM::loadMBC(ifstream *file) {

}

void ROM::checkCart(string path) {

}

int ROM::checkRomSize(int headerSize, int fileSize) {
	return 1;
}