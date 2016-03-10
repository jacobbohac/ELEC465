#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <sstream>
#include <utility>
#include <vector>

#define ALPHABET_SIZE 256

using namespace std;

struct CountStruct{
	int counts[ALPHABET_SIZE+1];
	int cumCounts[ALPHABET_SIZE+1];
	int totalCount;
};

char* inputBuffer;
int curr_byte_of_buffer = 1; // First one gets used in tag initialization
int curr_bit_of_byte = 0;
int numCharsToDecode = 0;

void decode(CountStruct* countStruct);
void parseFile(string infileName, CountStruct* countStruct);
void populateCountStruct(vector<pair<int,int>> headerValues, CountStruct* countStruct);
void printCountStruct(CountStruct* countStruct);
bool msb(uint8_t b);
bool getE3state(uint8_t l, uint8_t u);
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
	uint8_t l = 0;
	uint8_t u = 255;
	uint8_t t;
	uint8_t k;

	int totalCount = countStruct->totalCount;
	int* cum_count = countStruct->cumCounts;

	t = inputBuffer[0];
	cout << "\n\ntag: " << (int)t << " " << byte_to_binary(t) << endl;

	while(1){

		cout << "\n---------- loop --------------" << endl;
		k = 0;

		uint16_t first = t-l+1;
		uint16_t second = u-l+1;

		cout << ((first*totalCount-1)/second) << endl;
		while(((first*totalCount-1)/second) >= cum_count[k]){
			k++;
		}

		k--;

		// decode symbol x
		char x = (char)k;
		decodedString += x;
		cout << "\t\t Decoded String = " << decodedString << endl;
		if(decodedString.size() == numCharsToDecode){break;}

		uint16_t tmp = u-l+1;
		uint8_t l_cpy = l;
		l = l + (tmp*cum_count[x-1 + 1])/totalCount;
		u = l_cpy + ((tmp*cum_count[x + 1])/totalCount) - 1;


		
		cout << "l: " << byte_to_binary(l) << " - " << (int)l << endl;
		cout << "u: " << byte_to_binary(u) << " - " << (int)u << endl;
		cout << "t: " << byte_to_binary(t) << " - " << (int)t << endl;

		while((msb(l) == msb(u)) || getE3state(l,u)){
			cout << endl;
			if(msb(l) == msb(u)){
				// shift l by 1 bit left, and shift 0 into LSB
				l = l << 1;
				l = l & 254;

				// shift u by 1 bit left,  and shift 1 into LSB
				u = u << 1;
				u = u | 1;

				// shift tag to the left by 1 and shift in next bit from bitstream
				t = t << 1;
				t |= getNextBit();

				cout << "l: " << byte_to_binary(l) << " - " << (int)l << endl;
				cout << "u: " << byte_to_binary(u) << " - " << (int)u << endl;
				cout << "t: " << byte_to_binary(t) << " - " << (int)t << endl;
			}
			if(getE3state(l,u)){
				// shift l by 1 bit left, and shift 0 into LSB
				l = l << 1;
				l = l & 254;

				// shift u by 1 bit left,  and shift 1 into LSB
				u = u << 1;
				u = u | 1;

				// shift tag to the left by 1 and shift in next bit from bitstream
				// compliment (new) MSB of l, u, t

				t = t << 1;
				t |= getNextBit();

				l = l ^ 128;
				u = u ^ 128;
				t = t ^ 128;

				cout << "l: " << byte_to_binary(l) << " - " << (int)l << endl;
				cout << "u: " << byte_to_binary(u) << " - " << (int)u << endl;
				cout << "t: " << byte_to_binary(t) << " - " << (int)t << endl;
			}
		}
	}
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