#ifndef COMMAND_H
#define COMMAND_H

#include <csdtio>
#include <cstdlib>
#include <vector>

#include "token.h"

enum ERROR_STATE {
	NO_ERROR
}

char *errorStr = NULL;
ERROR_STATE errorNo = NO_ERROR;

bool handleCommand(vector<Token> tokens);


#endif /* COMMAND_H */