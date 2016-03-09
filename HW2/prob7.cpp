#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <fstream>

using namespace std;

int main(){

	int i,j,k;
	string str;
	srand (time(NULL));
	string line;
	int weights[26];
	double probs[26];
	int total = 0;
	ifstream file("4letters.word");
	int singleContextWeights[26][26];
	int doubleContextWeights[26][26][26];

	for(i=0; i<26; i++){
		for(j=0; j<26; j++){
			singleContextWeights[i][j] = 0;
		}
	}
	
	for(i=0; i<26; i++){
		for(j=0; j<26; j++){
			for(k=0; k<26; k++){
				doubleContextWeights[i][j][k] = 0;
			}
		}
	}

//------------------------------ PART A ------------------------------//

	cout << "\nPart (a) - Random 4 letter words\n" << endl;
	
	for(i=1; i<=100; i++){
		str = "";
		for(j=0; j<4; j++){
			str += rand() % 26 + 97;
		}
		cout << str << " ";
		if(!(i%10)) cout << endl;
	}

//------------------------------ PART B ------------------------------//

	cout << "\n\nPart (b) - 4 letter words from probability model\n" << endl;

	// obtain probability model for the alphabet
	for(i=0; i<26; i++){
		probs[i] = 0.0;
		weights[i] = 0;
	}


	// open file and populate 0,1,2 context probabilities
	if(file.is_open()){
		while(getline (file, line) ){
			// send string to lower
			for(i=0; i<4; i++){
				if(line.at(i) >= 65 && line.at(i) <= 90){
					line.at(i) = line.at(i) + 32;
				}
			}
			for(i=0; i<4; i++){
				char c = line.at(i);			
				if(c >= 97 && c <= 122){
					// no context
					weights[line.at(i) - 97] ++;
					// 1 letter context
					if(i<3) singleContextWeights[line.at(i)-97][line.at(i+1)-97]++;
					// 2 letter context					
					if(i<2) doubleContextWeights[line.at(i)-97][line.at(i+1)-97][line.at(i+2)-97]++;
				}
			}
			total += 4;
		}
	}

	file.close();
	for(i=0; i<26; i++){
		probs[i] = (double) weights[i] / (double) total;
	}

	// create cdf for 0 context
	double cdf[26];
	cdf[0] = probs[0];
	for(i=1; i<26; i++){
		cdf[i] = cdf[i-1] + probs[i];
	}

	for(i=1; i<=100; i++){
		str = "";
		for(j=0; j<4; j++){
			int k=0;
			double random = ((double) rand() / (RAND_MAX));
			while(random >= cdf[k]){
				k++;
			}
			str += (char)(97 + k); 
		}
		cout << str << " ";
		if(!(i%10)) cout << endl;
	}

//------------------------------ PART C ------------------------------//

	cout << "\n\nPart (c) - single letter context\n" << endl;
	
	// Create cdf for each letter
	double singleContextCdf[26][26];
	for(i=0; i<26; i++){
		int colTotal = 0;
		for(j=0; j<26; j++){
			colTotal += singleContextWeights[i][j];			
		}	
		singleContextCdf[i][0] = (double)singleContextWeights[i][0] / (double) colTotal;
		for(j=1; j<26; j++){
			singleContextCdf[i][j] = ((double)singleContextWeights[i][j] / (double) colTotal) + singleContextCdf[i][j-1];
		}
	}

	for(i=1; i<=100; i++){
		str = "";

		// get first letter from 0 context cdf
		int k=0;
		char firstLetterIndex;
		double random = ((double) rand() / (RAND_MAX));
		while(random >= cdf[k]){
			k++;
		}
		firstLetterIndex = k;
		str += (char)(97 + k); 

		// get letters 2,3,4
		char c = firstLetterIndex;
		for(j=1; j<4; j++){
			
			double random = ((double) rand() / (RAND_MAX));
			k=0;
			while(random >= singleContextCdf[c][k]){
				k++;
			}
			str += (char)(97+k);
			c = k;
		}
		cout << str << " ";
		if(!(i%10)) cout << endl;
	}

//------------------------------ PART D ------------------------------//

	cout << "\nPart (d) - two-letter context\n" << endl;
	
	// create 2 letter context cdf
	double doubleContextCdf[26][26][26];
	for(i=0; i<26; i++){
		for(j=0; j<26; j++){
			int colTotal = 0;

			for(k=0; k<26; k++){
				colTotal += doubleContextWeights[i][j][k];
			}

			if(colTotal == 0){
				doubleContextCdf[i][j][0] = 0.0;
				
			}
			else{
				doubleContextCdf[i][j][0] = (double)doubleContextWeights[i][j][0] / (double) colTotal;
				for(k=1; k<26; k++){
					double a = (double) doubleContextWeights[i][j][k];
					double b = (double) colTotal;
					double c = doubleContextCdf[i][j][k-1];
					doubleContextCdf[i][j][k] = (a/b) + c;
				}
			}	
					
		}						
			
	}

	for(i=1; i<=100; i++){
		str = "";
		// get first letter from 0 context cdf
		k=0; 
		char firstLetterIndex;
		double random = ((double) rand() / (RAND_MAX));
		
		while(random >= cdf[k]){
			k++;
		}
		firstLetterIndex = k;
		str += (char)(97 + k); 

		// get second letter from 1 context cdf
		random = ((double) rand() / (RAND_MAX));
		k=0;
		while(random >= singleContextCdf[firstLetterIndex][k]){
			k++;
		}	
		str += (char)(97+k);
		// get letters 3 and 4
		char a = str.at(0)-97;
		char b = str.at(1)-97;
		bool noContext = false;
		for(j=2; j<4; j++){
			double random = ((double) rand() / (RAND_MAX));
			k=0;
			while(random >= doubleContextCdf[a][b][k]){
				k++;
				if(k == 25){
					noContext = true;
					break;
				}
			}
			if(noContext){
				// going to use 0 context probabilty model
				random = ((double) rand() / (RAND_MAX));
				k=0;
				while(random >= cdf[k]){
					k++;
				}		
			}
			str += (char)(97+k);
			a = b;
			b = k;							
		}		

		cout << str << " ";
		if(!(i%10)) cout << endl;
	}

}
