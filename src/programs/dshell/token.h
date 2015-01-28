#ifndef TOKEN_H
#define TOKEN_H

#include <sstream>
#include <vector>

typedef struct {
	char *str;
	int length;
} Token;

bool tokenize(char *input, std::vector<Token> &tokens);

void addToQueue(std::stringstream &ss, std::vector<Token> &tokens);


#endif