#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cstring>

#define ALPHABET_SIZE 256

using namespace std;

int numBits = 0;
int bitBuffer = 0;
int totNumBytesOut = 0;

struct CountStruct{
	int counts[ALPHABET_SIZE+1];
	int cumCounts[ALPHABET_SIZE+1];
	int totalCount;
};

void parseInput(string infileName, CountStruct* countStruct);
void printArray(int* arr, int size);
void printCountStruct(CountStruct* countStruct);
void encode(CountStruct* countStruct, string infileName, string outfileName);
bool msb(uint8_t b);
bool getE3state(uint8_t l, uint8_t u);
void outputBit(bool bit_bool, ostream &outFile);
void writeHeader(string outfileName, CountStruct* countStruct);

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

int main(int argc, char* argv[]){

	/// Step 1: parse the input (input file, output file)
	if(argc != 3){
		cout << "USAGE: ./Encoder {infile} {outfile}" << endl;
		return -1;
	}

	string infileName = argv[1];
	string outfileName = argv[2];

	CountStruct* countStruct = new CountStruct;

	/// PASS 1 - read the counts
	parseInput(infileName, countStruct);

	/// write out a header
	writeHeader(outfileName, countStruct);

	/// PASS 2 - do the encoding
	encode(countStruct, infileName, outfileName);

	return 0;
}

void encode(CountStruct* countStruct, string infileName, string outfileName){
	cout << "encoding" << endl;

	char symbol;
	int total = countStruct->totalCount;
	int* cum_count = countStruct->cumCounts;
	int scale3 = 0;
	bool b;	

	ofstream outFile(outfileName, std::ios::app);	
	ifstream inFile(infileName, std::ios::in | std::ios::binary);
	

	uint8_t l = 0;
	uint8_t u = 255;

	while(!inFile.eof()){
		inFile.get(symbol);

		//cout << "symbol = " << symbol << endl;

		l = l + ((u-l+1) * cum_count[symbol - 1]) / total;
		u = l + (((u-l+1) * cum_count[symbol]) / total) - 1;

		//cout << "\tl = " << byte_to_binary(l) << endl;
		//cout << "\tu = " << byte_to_binary(u) << endl;

		bool u_msb = msb(u);
		bool l_msb = msb(l);
		bool e3 = getE3state(l,u);

		//cout << "\tl_msb = " << l_msb << endl;
		//cout << "\tu_msb = " << u_msb << endl;
		//cout << "\te3 = " << e3 << endl;

		while((u_msb == l_msb) || e3){
			if(u_msb == l_msb){
				b = u_msb;

				//cout << "\tMatch, b = " << b << endl;

				outputBit(b, outFile);

				// shift l by 1 bit left, and shift 0 into LSB
				l = l << 1;
				l = l | 1;

				// shift u by 1 bit left,  and shift 1 into LSB
				u = u << 1;
				u = u & 254;

				//cout << "\tl = " << byte_to_binary(l) << endl;
				//cout << "\tu = " << byte_to_binary(u) << endl;

				while(scale3 > 0){					
					outputBit(!b, outFile);
					scale3--;
				}
				
				u_msb = msb(u);
				l_msb = msb(l);

				//cout << "\tl_msb = " << l_msb << endl;
				//cout << "\tu_msb = " << u_msb << endl;
				//cout << endl;
			}
			if(e3){
				// shift l by 1 bit left, and shift 0 into LSB
				l = l << 1;
				l = l | 1;
				// shift u by 1 bit left,  and shift 1 into LSB
				u = u << 1;
				u = u & 254;

				// compliment (new) MSB of l and u
				l = l ^ 128;
				u = u ^ 128;

				scale3 ++;
			}
		}
	}
}

void writeHeader(string outfileName, CountStruct* countStruct){
		ofstream outFile(outfileName);
		int i;
		for(i=0; i<ALPHABET_SIZE; i++){
			if(countStruct->counts[i] > 0){				
				outFile << i << " " << countStruct->counts[i] << "\n";				
			}
		}
		outFile.close();
}

bool msb(uint8_t b){
	if((b >> 7) == 1){
		return true;
	}else if ((b >> 7) == 0){
		return false;	
	}else{
		cout << "MSB resulted in a value greater not 0 or 1" << endl;
		throw std::exception();
	}
	return false;
}

bool getE3state(uint8_t l, uint8_t u){
	// if l = 01..., u = 10....
	if((l >> 6) == 1 && (u >> 6) == 2){
		return true;
	}
	return false;
}

void outputBit(bool bit_bool, ostream &outFile){
	
	int bit = 0;
	if(bit_bool) bit = 1;

	bitBuffer |= (bit << numBits);
	numBits++;

	if(numBits == 8){

		outFile << reverse(bitBuffer);
		cout << byte_to_binary(reverse(bitBuffer)) << " ";
		numBits = 0;
		bitBuffer = 0;

		totNumBytesOut++;
	}
}

void printCountStruct(CountStruct* countStruct){
	int i;
	for(i=0; i<ALPHABET_SIZE+1; i++){
		cout << i << ":\t" << countStruct->counts[i] << "\t" << countStruct->cumCounts[i] << endl;
	}
	cout << "TotalCount = " << countStruct->totalCount << endl;
}

void parseInput(string infileName, CountStruct* countStruct){
	
	cout << "infile = " << infileName << endl;
	ifstream inFile(infileName, std::ios::in | std::ios::binary);
	char c;

	while(!inFile.eof()){
		inFile.get(c);
		cout << c;
		countStruct->counts[c]++;
		countStruct->totalCount++;
	}

	// Calculate CumCounts
	int i;
	countStruct->cumCounts[0] = 0;
	for(i=1; i<ALPHABET_SIZE+1; i++){
		countStruct->cumCounts[i] = countStruct->cumCounts[i-1] + countStruct->counts[i-1];
	}
}

void printArray(int* Counts, int size){
	int i;
	for(i=0; i<size; i++){
		cout << i << ": " << Counts[i] << endl;
	}
}