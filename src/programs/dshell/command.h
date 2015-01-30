#ifndef COMMAND_H
#define COMMAND_H

#include <cstdio>
#include <cstdlib>
#include <vector>

#include "token.h"
#include "job.h"

enum ERROR_STATE {
	NO_ERROR
};

extern char *errorStr;
extern ERROR_STATE errorNo;

bool handleCommand(std::vector<Token> tokens, std::vector<char *> &jobs);


#endif /* COMMAND_H */