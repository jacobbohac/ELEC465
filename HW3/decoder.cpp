#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <bitset>
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

int inputFileSize;
int startOfData;

void parseInput(char* fileName, int* codeLengths);
void printArray(int *A, int length);
void getCanonicalValues(codeStruct* codeStructs, ByteLengthPair* byteLengthPairs);
void getByteValuePairs(int* codeLengths, ByteLengthPair* byteLengthePairs);
bool isLengthValue(int length, int value, codeStruct* codeStructs);
char lengthValue(int length, int value, codeStruct* codeStructs);
int getBits(int start, int length);
bool bcompare(ByteLengthPair lhs, ByteLengthPair rhs){ return lhs.codeLength > rhs.codeLength; }

char *compressedData;
int maxCodeWordLength = 0;

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

int main(int argc, char* argv[]){

	if(argc != 3){
		cout << "USAGE: ./decoder {input file} {output file}" << endl;
		return 0;
	}

	int codeLengths[ALPHABET_SIZE];
	parseInput(argv[1], codeLengths);

	// populate a struct with
	// {byte value, length}
	ByteLengthPair byteLengthPairs[ALPHABET_SIZE];
	getByteValuePairs(codeLengths, byteLengthPairs);

	int i;

	// sort the byteLengthPairs by length
	sort(byteLengthPairs, byteLengthPairs+ALPHABET_SIZE, bcompare);

	/// generate the canonical codes
	codeStruct codeStructs[ALPHABET_SIZE];
	getCanonicalValues(codeStructs, byteLengthPairs);

	unsigned int curr = 0;

	ofstream outFile;
	outFile.open(argv[2]);

	int j;
	int length = 0;
	for(i=startOfData; i<=inputFileSize; i++){

		unsigned char c = compressedData[i];

		for(j=1; j<=8; j++){

			length++;
			unsigned char new_c = 0;
			new_c = (c >> (8-j));
			curr = (curr << 1);

			curr |= new_c;

			if(isLengthValue(length, curr, codeStructs)){

				if(j == 7){
					c &= 0x01;
				}
				if(j == 6){
					c &= 0x03;
				}
				if(j == 5){
					c &= 0x07;
				}
				if(j == 4){
					c &= 0x0f;
				}
				if(j == 3){
					c &= 0x1f;
				}
				if(j == 2){
					c &= 0x3f;
				}
				if(j == 1){
					c &= 0x7f;
				}
				if(j == 8){
					c &= 0x00;
				}

				outFile << lengthValue(length, curr, codeStructs);

				curr = 0;
				length = 0;
				
			}
		}	
	}
}


bool isLengthValue(int length, int value, codeStruct* codeStructs){
	int i;
	for(i=0; i<ALPHABET_SIZE; i++){
		if(codeStructs[i].value == value && codeStructs[i].length == length){
			return true;
		}
	}
	return false;
}

char lengthValue(int length, int value, codeStruct* codeStructs){
	int i;
	for(i=0; i<ALPHABET_SIZE; i++){
		if(codeStructs[i].value == value && codeStructs[i].length == length){
			return i;
		}
	}
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

void parseInput(char* fileName, int* codeLengths){
	
	char c;
	string s;
	ifstream infile(fileName);
	stringstream buffer;

	buffer << infile.rdbuf();
	int i;

	startOfData = 0;

	for(i=0; i<ALPHABET_SIZE; i++){
		getline(buffer, s, ' ');
		startOfData += s.length() + 1;
		codeLengths[i] = atoi(s.c_str());

		if(codeLengths[i] > maxCodeWordLength){
			maxCodeWordLength = codeLengths[i];
		}
	}

	s = buffer.str();

	compressedData = (char*)malloc(sizeof(char) * (s.length()+1));


	for(i=0; i<s.length(); i++){
		compressedData[i] = s[i];
	}
	
	inputFileSize = i;
}

void printArray(int *A, int length){
	int i;
	for(i=0; i < length; i++){
		cout << A[i] << " ";
	}
	cout << endl << endl;
	return;
}
