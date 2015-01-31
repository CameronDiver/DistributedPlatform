#ifndef COMMAND_H
#define COMMAND_H

#include <cstdio>
#include <cstdlib>
#include <vector>

#include "common.h"
#include "job.h"
#include "token.h"

enum ERROR_STATE {
	NO_ERROR
};

extern char *errorStr;
extern ERROR_STATE errorNo;

bool handleCommand(std::vector<Token> tokens, std::vector<char *> &jobs);


#endif /* COMMAND_H */
