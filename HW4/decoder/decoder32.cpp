#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <sstream>
#include <utility>
#include <vector>
#include <stdexcept>

#define ALPHABET_SIZE 256

using namespace std;

struct CountStruct{
	int counts[ALPHABET_SIZE+1];
	int cumCounts[ALPHABET_SIZE+1];
	int totalCount;
};

char* inputBuffer;
int curr_byte_of_buffer = 4; // First 4 gets used in tag initialization
int curr_bit_of_byte = 0;
int numCharsToDecode = 0;

void decode(CountStruct* countStruct);
void parseFile(string infileName, CountStruct* countStruct);
void populateCountStruct(vector<pair<int,int>> headerValues, CountStruct* countStruct);
void printCountStruct(CountStruct* countStruct);
bool msb(uint32_t b);
bool getE3state(uint32_t l, uint32_t u);
bool getNextBit();
string decodedString;

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
	cout << "DECODER" << endl;

	if(argc != 3){
		cout << "USAGE: ./Encoder {infile} {outfile}" << endl;
		return -1;
	}

	string infileName = argv[1];
	string outfileName = argv[2];

	CountStruct* countStruct = new CountStruct;

	parseFile(infileName, countStruct);

	decode(countStruct);
}

void parseFile(string infileName, CountStruct* countStruct){
	vector<pair<int,int>> headerValues;

	ifstream infile(infileName, std::ios::in | std::ios::binary);	

	string line;
	string dataString;
	while (getline(infile, line))
	{
	    std::istringstream iss(line);
	    cout << "-> " << line << endl;
	    int a, b;
	    if (iss >> a >> b) {
	    	headerValues.push_back(make_pair(a,b));
	    	cout << "a: " << a << "b: " << b << endl;	    
	    }else{
	    	iss >> a;
	    	numCharsToDecode = a;

	    	getline(infile, line);
	    	std::istringstream iss(line);
	    	cout << line << endl;
	    	dataString = line;
	    }	   
	}

	inputBuffer = new char[dataString.size()+1];
	inputBuffer[dataString.size()] = 0;

	memcpy(inputBuffer, dataString.c_str(), dataString.size());

	populateCountStruct(headerValues, countStruct);
}

void populateCountStruct(vector<pair<int,int>> headerValues, CountStruct* countStruct){
	cout << "populating countStruct" << endl;
	int i;

	for(i = 0; i<headerValues.size(); i++){
		cout << headerValues[i].first << " " << headerValues[i].second << endl;
		countStruct->counts[headerValues[i].first] = headerValues[i].second;
	}

	// Calculate CumCounts
	countStruct->cumCounts[0] = 0;
	for(i=1; i<ALPHABET_SIZE+1; i++){
		countStruct->cumCounts[i] = countStruct->cumCounts[i-1] + countStruct->counts[i-1];
	}
	countStruct->totalCount = countStruct->cumCounts[ALPHABET_SIZE];
}

void printCountStruct(CountStruct* countStruct){
	int i;
	for(i=0; i<ALPHABET_SIZE+1; i++){
		cout << i << ":\t" << countStruct->counts[i] << "\t" << countStruct->cumCounts[i] << endl;
	}
	cout << "TotalCount = " << countStruct->totalCount << endl;
}

bool getNextBit(){
	// There will be an exception when we reach the end of the buffer

	char c = inputBuffer[curr_byte_of_buffer];
	bool bit = (c >> (7-curr_bit_of_byte)) & 1;

	curr_bit_of_byte++;

	if(curr_bit_of_byte == 8){
		curr_byte_of_buffer++;
		curr_bit_of_byte = 0;
	}
	return bit;
}

void decode(CountStruct* countStruct){
	uint32_t l = 0;
	uint32_t u = 4294967295;
	uint32_t t;
	uint8_t k;

	int totalCount = countStruct->totalCount;
	int* cum_count = countStruct->cumCounts;

	t = (uint32_t)(inputBuffer[0]&255) << 24 |
      (uint32_t)(inputBuffer[1]&255) << 16 |
      (uint32_t)(inputBuffer[2]&255) << 8  |
      (uint32_t)(inputBuffer[3]&255);

	cout << "\n\ntag: " << t << " " << int_to_binary(t) << endl;

	while(1){

		cout << "\n---------- loop --------------" << endl;
		k = 0;

		cout << "l: " << int_to_binary(l) << " - " << l << endl;
		cout << "u: " << int_to_binary(u) << " - " << u << endl;
		cout << "t: " << int_to_binary(t) << " - " << t << endl;

		uint64_t first = (uint64_t)t-(uint64_t)l+1;
		uint64_t second = (uint64_t)u-(uint64_t)l+1;

		while(((first*totalCount-1)/second) >= cum_count[k]){
			k++;
		}

		k--;

		// decode symbol x
		char x = (char)k;
		decodedString += x;
		cout << "\t\t Decoded String = " << decodedString << endl;
		if(decodedString.size() == numCharsToDecode){break;}

		uint64_t tmp = (uint64_t)u-(uint64_t)l+1;
		uint32_t l_cpy = l;
		cout << "tmp = " << tmp << endl;

		l = l + (tmp*cum_count[x-1 + 1])/totalCount;
		u = l_cpy + ((tmp*cum_count[x + 1])/totalCount) - 1;


		
		cout << "l: " << int_to_binary(l) << " - " << l << endl;
		cout << "u: " << int_to_binary(u) << " - " << u << endl;
		cout << "t: " << int_to_binary(t) << " - " << t << endl;

		cout << "msb(l) = " << msb(l) << endl;
		cout << "msb(u) = " << msb(u) << endl;
		cout << "estate = " << getE3state(l,u) << endl;

		while((msb(l) == msb(u)) || getE3state(l,u)){
			cout << endl;
			if(msb(l) == msb(u)){
				cout << "E1,2" << endl;
				// shift l by 1 bit left, and shift 0 into LSB
				l = l << 1;
				l = l & 4294967294;

				// shift u by 1 bit left,  and shift 1 into LSB
				u = u << 1;
				u = u | 1;	

				// shift tag to the left by 1 and shift in next bit from bitstream
				t = t << 1;
				t |= getNextBit();

				cout << "l: " << int_to_binary(l) << " - " << l << endl;
				cout << "u: " << int_to_binary(u) << " - " << u << endl;
				cout << "t: " << int_to_binary(t) << " - " << t << endl;
			}
			if(getE3state(l,u)){
				cout << "E3" << endl;
				// shift l by 1 bit left, and shift 0 into LSB
				l = l << 1;
				l = l & 4294967294;

				// shift u by 1 bit left,  and shift 1 into LSB
				u = u << 1;
				u = u | 1;	

				// shift tag to the left by 1 and shift in next bit from bitstream
				// compliment (new) MSB of l, u, t

				t = t << 1;
				t |= getNextBit();

				l = l ^ 2147483648;
				u = u ^ 2147483648;
				t = t ^ 2147483648;

				cout << "l: " << byte_to_binary(l) << " - " << l << endl;
				cout << "u: " << byte_to_binary(u) << " - " << u << endl;
				cout << "t: " << byte_to_binary(t) << " - " << t << endl;
			}
		}
	}
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