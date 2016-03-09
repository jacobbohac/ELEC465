#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <iomanip>
#include <string>
#include <bitset>

#define ALPHABET_SIZE 256

using namespace std;

int Acopy[ALPHABET_SIZE];

void parseFile(char* fileName, int* chars);
void calc_huff_lens(int* A, int length);
void printArray(int* A, int length);
void sortArray(int *A, int length, int* perm);
void getPermArray(int *perm, int* chars, int length);
void printTable(int* chars, int* perm, int length);
void writeLengths(int* chars, int* perm, int length, char* fileName);
void getCodes(int* chars, string* codes, int length);

int main(int argc, char* argv[]){

	if(argc != 3){
		cout << "USAGE: ./codes {input file} {output file}" << endl;
		return 0;
	}

	int chars[ALPHABET_SIZE] = {0};
	int perm[ALPHABET_SIZE];
	parseFile(argv[1], chars);

	getPermArray(perm, chars, ALPHABET_SIZE);

	sortArray(chars, ALPHABET_SIZE, perm);	

	calc_huff_lens(chars, ALPHABET_SIZE);

	printTable(chars, perm, ALPHABET_SIZE);

	writeLengths(chars, perm, ALPHABET_SIZE, argv[2]);

	return 0;
}

void parseFile(char* fileName, int* chars){
	char c;
	ifstream infile(fileName);

	if(infile.is_open()){
		while(infile.get(c)){
			chars[c]++;	
		}
	}
}

// This function is adopted from A. Moffat's psuedocode
void calc_huff_lens(int* A, int length){

	int leaf = 0;
	int root = 0;
	int next = 0;
	int n = length;
	int avail;
	int used;
	int depth;

	/// PHASE 1
	for(next = 0; next <= n-2; next++){
		
		if(leaf >= n || (root < next && A[root] < A[leaf])){
			A[next] = A[root];
			A[root] = next;
			root++;
		}else{
			A[next] = A[leaf];
			leaf++;
		}

		if(leaf >= n || (root < next && A[root] < A[leaf])){
			A[next] += A[root];
			A[root] = next;
			root++;
		}else{
			A[next] += A[leaf];
			leaf++;
		}		
	}

	/// PHASE 2
	A[n-2] = 0;
	for(next = n-3; next >= 0; next--){
		// Implement Alistair's career highlight
		A[next] = A[A[next]] + 1;
	}

	/// PHASE 3
	avail = 1;
	used = 0;
	depth = 0;
	root = n-2;
	next = n-1;

	while(avail > 0){
		while(root >= 0 && A[root] == depth){
			used++;
			root--;
		}
		while(avail > used){
			A[next] = depth;
			next--;
			avail--;
		}
		avail = 2 * used;
		depth++;
		used = 0;
	}

	return;
}

bool order(int i, int j){
	return Acopy[i] < Acopy[j];
}

void sortArray(int* A, int length, int* perm){
	sort(perm, perm + length, order);
	sort(A, A + length);
}

void printArray(int *A, int length){
	int i;
	for(i=0; i < length; i++){
		cout << A[i] << " ";
	}
	cout << endl << endl;
	return;
}

void getPermArray(int* perm, int* chars, int length){
	int i;
	for(i=0; i<length; i++){
		perm[i] = i;
	}
	for(i=0; i<length; i++){
		Acopy[i] = chars[i];
	}
}

void getCodes(int* chars, string* codes, int length){
	
	int i;
	int prev = 0;


	for(i=0; i < length; i++){

		if(i>0){
			if(chars[i-1] > chars[i]){
				// previous length was longer
				prev = (prev >> chars[i-1] - chars[i]);
			}
		}

		string s = bitset<32>(prev).to_string();
		codes[i] = s.substr(32 - chars[i],chars[i]);

		prev++;
	}
}

void printTable(int* chars, int* perm, int length){
	int i;
	int width = 8;
	char separator = ' ';

	string codes[ALPHABET_SIZE];
	getCodes(chars, codes, ALPHABET_SIZE);


	cout << left << setw(width) << setfill(separator) << "value";
	cout << left << setw(width) << setfill(separator) << "char";
    cout << left << setw(width) << setfill(separator) << "len";
    cout << left << setw(20) << setfill(separator) << "code";
    cout << endl;

    int j=0;

	for(i=0; i < length; i++, j++){
		cout << left << setw(width) << setfill(separator) <<  perm[i];
		if(perm[i] > 32){
			cout << left << setw(width) << setfill(separator) <<  (char)perm[i];
		}else{
			cout << left << setw(width) << setfill(separator) <<  perm[i];
		}
		
    	cout << left << setw(width) << setfill(separator) << chars[i];
    	cout << left << setw(20) << setfill(separator) << codes[i];
    	cout << endl;
	}
}

void writeLengths(int* chars, int* perm, int length, char* fileName){
	int i,j;

	ofstream outFile(fileName);

	for(i=0; i<length; i++){
		for(j=0; j<length; j++){
			if(i == perm[j]){
				outFile << chars[j] << ' ';
				break;
			}
		}
	}
}
