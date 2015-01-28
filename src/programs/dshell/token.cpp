#include "token.h"

#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>

const int delimiterCount = 3;
const char delimiters[] = {
	' ', 	// space, standard delimeter

	'&',	// and, run the next process if the first completed
			// successfully
	
	'|',	// pipe, output of the first process fed into the second

	'\n'
			
};

bool tokenize(char *input, std::vector<Token> &tokens) {
	// state variables
	bool inString = false;
	bool inEquation = false;

	// stringbuilder
	std::stringstream ss;

	int length = strlen(input);
	for(int i = 0; i < length; ++i) {
		// get the current character
		char c = input[i];
		

		if(inString) {
			if(c == '"') {
				inString = false;
				addToQueue(ss, tokens);
			} else {
				ss << c;
			}
		} else if(inEquation) {

		} else {
			switch(c) {
				// check for equation
				case '$':
					//TODO
				break;
				// check for string
				case '"':
					if(c == '"'){
						// FIXME: handle escaping
						
						// store the quote?
						inString = true;
						addToQueue(ss, tokens);
					}
				break;
				// check for delimiter
				default:
				{ 
					bool delim = false;
					for(int j = 0; j < delimiterCount; ++j) {
						if(c == delimiters[j]){
							// if it's not a space or nl save it as a token
							addToQueue(ss, tokens);
							if(c != ' ' && c != '\n'){
								ss << c;
								addToQueue(ss, tokens);
							}
							delim = true;
							break;
						}
					}

					if(!delim)
						ss << c;
				}
				break;
			}

		}
	}

	// if there is something left in the buffer, add it to the 
	// output array
	addToQueue(ss, tokens);

	return true;
}

inline int ssSize(std::stringstream &ss){
	ss.seekg(0, std::ios::end);
	int size = ss.tellg();
	ss.seekg(0, std::ios::beg);

	return size;
}

void addToQueue(std::stringstream &ss, std::vector<Token> &tokens) {
	// if the string stream is more than just the null byte
	int size = ssSize(ss);
	if(size > 0) {
		// add it to the output queue
		Token t;
		t.str = (char *)malloc(size);
		strcpy(t.str, ss.str().c_str());
		t.length = strlen(t.str);

		ss.str("");

		tokens.push_back(t);
	} 
		
}