#include "command.h"

#include <sstream>
#include <cstring>

char jobSplitters[] = { '&' };
int jobSplittersSize = sizeof(jobSplitters)/sizeof(jobSplitters[0]);


char *errorStr = NULL;
ERROR_STATE errorNo = NO_ERROR;

bool handleCommand(std::vector<Token> tokens, std::vector<char *> &jobs) {
	std::stringstream ss;
	// go through all of the tokens and calculate how many jobs there are
	for(Token t : tokens) {
		// change this if there is ever 2 character operators
		if(t.length == 1) {
			bool splitter = false;
			for(char c : jobSplitters){
				if(t.str[0] == c) {
					splitter = true;
					const char *str = ss.str().c_str();
					int len = strlen(str);

					char *temp = (char *)malloc(len + 1);
					strcpy(temp, str);

					jobs.push_back(temp);
					ss.str("");
				}
			}
			if(!splitter) goto add;

		} else {
			add:
			// TODO: check that it's not something else that's special
			ss << t.str << ' ';
		}
	}

	if(strlen(ss.str().c_str()) > 0){
		const char *str = ss.str().c_str();
		int len = strlen(str);

		char *temp = (char *)malloc(len + 1);
		strcpy(temp, str);

		jobs.push_back(temp);
	}

	return true;
}