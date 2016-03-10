#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <vector>
#include <stdexcept>

#define ALPHABET_SIZE 256

using namespace std;

int numBits = 0;
int bitBuffer = 0;
int totNumBytesOut = 0;
vector<int> outputBits;

struct CountStruct{
	int counts[ALPHABET_SIZE+1];
	int cumCounts[ALPHABET_SIZE+1];
	int totalCount;
};

void parseInput(string infileName, CountStruct* countStruct);
void printArray(int* arr, int size);
void printCountStruct(CountStruct* countStruct);
void encode(CountStruct* countStruct, string infileName, ostream &outFile);
bool msb(uint32_t b);
bool getE3state(uint32_t l, uint32_t u);
void outputBit(bool bit_bool, ostream &outFile);
void writeHeader(string outfileName, CountStruct* countStruct);

unsigned char reverse(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

const char *byte_to_binary(int x){
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}

const char *int_to_binary(int x){
    static char b[33];
    b[0] = '\0';

    uint32_t z;
    for (z = 2147483648; z > 0; z >>= 1)
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
	ofstream outFile(outfileName, std::ios::app);	

	CountStruct* countStruct = new CountStruct;

	/// PASS 1 - read the counts
	parseInput(infileName, countStruct);

	/// write out a header
	writeHeader(outfileName, countStruct);

	/// PASS 2 - do the encoding
	encode(countStruct, infileName, outFile);

	/// FLUSH THE OUTPUT BUFFER
	while(numBits != 0){
		cout << "0";
		outputBit(0, outFile);
	}

	// OUTPUT THE BYTES FOR DEBUGGING
	int i;
	cout << "\noutputBits size: " << outputBits.size() << endl;
	for(i=0; i<outputBits.size(); i++){
		if(i%8 == 0 && i!=0) cout << " ";
		cout << outputBits.at(i);
	}
	cout << endl;	

	return 0;
}

void encode(CountStruct* countStruct, string infileName, ostream &outFile){
	cout << "encoding" << endl;

	char symbol;
	int total = countStruct->totalCount;
	int* cum_count = countStruct->cumCounts;
	uint scale3 = 0;
	
	ifstream inFile(infileName, std::ios::in | std::ios::binary);
	

	uint32_t l = 0;
	uint32_t u = 4294967295;

	cout << "\tl = " << int_to_binary(l) << " - " << l << endl;
	cout << "\tu = " << int_to_binary(u) << " - " << u << endl;

	while(!inFile.eof()){
		inFile.get(symbol);

		cout << symbol << endl;

		uint32_t l_cpy = l;

		cout << "\tl = " << int_to_binary(l) << " - " << l << endl;
		cout << "\tu = " << int_to_binary(u) << " - " << u << endl;


		cout << "u-l+1 " << (uint64_t)u-(uint64_t)l+1 << endl;
		cout << "total " << total << endl;

		l = (uint64_t)l + (((uint64_t)u-(uint64_t)l+1) * cum_count[symbol - 1 + 1]) / total;
		u = (uint64_t)l_cpy + ((((uint64_t)u-(uint64_t)l_cpy+1) * cum_count[symbol + 1]) / total) - 1;
	
		cout << "\tl = " << int_to_binary(l) << " - " << l << endl;
		cout << "\tu = " << int_to_binary(u) << " - " << u << endl;

		if(l > u){
			cout << cum_count[symbol - 1 + 1] << endl;
			cout << cum_count[symbol + 1] << endl;
			throw std::runtime_error("l > u");
		}

		while((msb(u) == msb(l)) || getE3state(l,u)){
			if(msb(u) == msb(l)){
				//cout << "\tMatch, b = " << b << endl;

				outputBit(msb(u), outFile);
				while(scale3 > 0){
					cout << "scale3" << endl;					
					outputBit(!msb(u), outFile);
					scale3--;
				}

				// shift l by 1 bit left, and shift 0 into LSB
				l = l << 1;
				l = l & 4294967294;

				// shift u by 1 bit left,  and shift 1 into LSB
				u = u << 1;
				u = u | 1;				

				cout << "\tl = " << int_to_binary(l) << " - " << l << endl;
				cout << "\tu = " << int_to_binary(u) << " - " << u << endl;

			}
			if(getE3state(l,u)){
				cout << "E3 Mapping" << endl;
				// shift l by 1 bit left, and shift 0 into LSB
				l = l << 1;
				l = l & 4294967294;

				// shift u by 1 bit left,  and shift 1 into LSB
				u = u << 1;
				u = u | 1;

				// compliment (new) MSB of l and u
				l = l ^ 2147483648;
				u = u ^ 2147483648;

				cout << "\tl = " << int_to_binary(l) << " - " << l << endl;
				cout << "\tu = " << int_to_binary(u) << " - " << u << endl;
				
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
		// write # of chars encoded
		outFile << countStruct->totalCount << "\n";

		outFile.close();
}

bool msb(uint32_t b){
	if((b >> 31) == 1){
		return true;
	}else if ((b >> 31) == 0){
		return false;	
	}else{		
		throw std::runtime_error("MSB resulted in a value greater not 0 or 1");
	}
	return false;
}

bool getE3state(uint32_t l, uint32_t u){
	// if l = 01..., u = 10....
	if((l >> 30) == 1 && (u >> 30) == 2){
		return true;
	}
	return false;
}

void outputBit(bool bit_bool, ostream &outFile){
	
	int bit = 0;
	if(bit_bool) bit = 1;

	cout << "\t\ttransmit " << bit << endl;
	outputBits.push_back(bit);

	bitBuffer |= (bit << numBits);
	numBits++;

	if(numBits == 8){

		outFile << reverse(bitBuffer);
		//cout << byte_to_binary(reverse(bitBuffer)) << " ";
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
	
	ifstream inFile(infileName, std::ios::in | std::ios::binary);
	char c;

	while(!inFile.eof()){
		inFile.get(c);
		if(!inFile.eof()){
			countStruct->counts[c]++;
			countStruct->totalCount++;
		}
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