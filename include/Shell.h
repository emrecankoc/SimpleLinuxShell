
#include "Tokenizer.h"
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>

int bgProcessCount = 0;
bool quitCommand = false;

void sigchld_handler(int sig);
void shell();
bool isFileExist(char*);
