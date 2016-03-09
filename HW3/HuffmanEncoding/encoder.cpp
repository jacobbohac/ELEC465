#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <string.h>

#define ALPHABET_SIZE 256

using namespace std;

struct codeStruct{
	int length;
	int value;
};

struct ByteLengthPair{
	int byteValue;
	int codeLength;
};

void readCodewordLengths(char* fileName, int* codeLengths);
void printArray(int* codeLengths, int length);
void readInputFile(char* fileName, vector<char> &fileBytes);
void printVector(vector<char> &fileBytes);
void writeHeader(int* codeLengths, char* fileName);
void writeFile(int* codeLengths, char* fileName, vector<char> &fileBytes, codeStruct* codeStructs);
void outputBit(int bit, ostream &outFile);
void encodeChar(int codeLength, int codeValue, ostream &outFile);
void getCanonicalValues(codeStruct* codeStructs, ByteLengthPair* byteLengthPairs);
void getByteValuePairs(int* codeLengths, ByteLengthPair* byteLengthePairs);

bool bcompare(ByteLengthPair lhs, ByteLengthPair rhs){ return lhs.codeLength > rhs.codeLength; }

unsigned char reverse(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

const char *byte_to_binary(int x)
{
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}

// global variable for the bit buffer
char bitBuffer;
int numBits;
int totNumBytesOut = 0;

int main(int argc, char* argv[]){

	if(argc != 4){
		cout << "USAGE: ./encoder {codeword lengths file} {input file} {output file}" << endl;
		return 0;		
	}

	/// Read codeword lengths from input file
	int codeLengths[ALPHABET_SIZE];
	readCodewordLengths(argv[1], codeLengths);

	// codeLengths = [20, 20, ..., 5]
	
	// populate a struct with
	// {byte value, length}
	ByteLengthPair byteLengthPairs[ALPHABET_SIZE];
	getByteValuePairs(codeLengths, byteLengthPairs);


	// sort the byteLengthPairs by length
	sort(byteLengthPairs, byteLengthPairs+ALPHABET_SIZE, bcompare);

	/// generate the canonical codes
	codeStruct codeStructs[ALPHABET_SIZE];
	getCanonicalValues(codeStructs, byteLengthPairs);

	/// Encode the other input file
	vector<char> fileBytes;
	readInputFile(argv[2], fileBytes);

	/// Write out a file with lengths in the header, and the rest following
	writeHeader(codeLengths, argv[3]);
	writeFile(codeLengths, argv[3], fileBytes, codeStructs);


	//cout << "tot output bytes = " << totNumBytesOut << endl;

}

void getByteValuePairs(int* codeLengths, ByteLengthPair* byteLengthPairs){
	int i;
	for(i=0; i<ALPHABET_SIZE; i++){
		byteLengthPairs[i].byteValue = i;
		byteLengthPairs[i].codeLength = codeLengths[i];
	}
}

void getCanonicalValues(codeStruct* codeStructs, ByteLengthPair* byteLengthPairs){
	int i;
	int prev = 0;

	for(i=0; i<ALPHABET_SIZE; i++){
		if(i>0){
			if(byteLengthPairs[i-1].codeLength > byteLengthPairs[i].codeLength){
				prev = (prev >> byteLengthPairs[i-1].codeLength - byteLengthPairs[i].codeLength);
			}
		}

		codeStructs[byteLengthPairs[i].byteValue].length = byteLengthPairs[i].codeLength;
		codeStructs[byteLengthPairs[i].byteValue].value = prev;

		prev++;
	}
}

void outputBit(int bit, ostream &outFile){
	
	bitBuffer |= (bit << numBits);
	numBits++;

	if(numBits == 8){

		outFile << reverse(bitBuffer);
		numBits = 0;
		bitBuffer = 0;



		totNumBytesOut++;
	}
}

void encodeChar(int codeLength, int codeValue, ostream &outFile){

	int i;
	for(i=codeLength-1; i>=0; i--){
		int mask = 0;
		mask = 1 << i;
		int masked_value = codeValue & mask;
		int bit = masked_value >> i;
		outputBit(bit, outFile);
	}
}

void writeFile(int* codeLengths, char* fileName, vector<char> &fileBytes, codeStruct* codeStructs){

	ofstream outFile;
	outFile.open(fileName, ios::app);

	bitBuffer = 0;
	numBits = 0;

	int i;
	for(i=0; i<fileBytes.size(); i++){	
		encodeChar(codeStructs[fileBytes[i]].length, codeStructs[fileBytes[i]].value, outFile);
	}

	outFile.close();
}

void readInputFile(char* fileName, vector<char> &fileBytes){

	char c;
	ifstream inFile(fileName);

	if(inFile.is_open()){
		while(inFile.get(c)){
			fileBytes.push_back(c);			
		}
	}
}

void writeHeader(int* codeLengths, char* fileName){

	ofstream outFile(fileName);

	int i;
	for(i=0; i<ALPHABET_SIZE; i++){
		outFile << codeLengths[i] << " ";
	}

	outFile.close();
}

void readCodewordLengths(char* fileName, int* codeLengths){
	char c;
	string s;
	ifstream infile(fileName);
	stringstream buffer;

	buffer << infile.rdbuf();
	int i;

	for(i=0; i<ALPHABET_SIZE; i++){
		getline(buffer, s, ' ');
		codeLengths[i] = atoi(s.c_str());
	}
}

void printArray(int *A, int length){
	int i;
	for(i=0; i < length; i++){
		cout << A[i] << " ";
	}
	cout << endl << endl;
	return;
}

void printVector(vector<char> &fileBytes){
	int i;
	for(i=0; i<fileBytes.size(); i++){
		cout << fileBytes[i];
	}
}