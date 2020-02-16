
#include "Tokenizer.h"
/*
	Tokenize the command and returns token count
 */

int tokenize_command(char *buff, char *tokens[])
{
	
	int token_count = 0;
	_Bool in_token = false;
	int num_chars = strnlen(buff, COMMAND_LENGTH);

	for (int i = 0; i < num_chars; i++) {
		switch (buff[i]) {
			
		case ' ':
		case '\t':
		case '\n':
			buff[i] = '\0';	
			in_token = false;
			break;
			
		default:
			if (!in_token) {
				tokens[token_count] = &buff[i];
				token_count++;
				in_token = true;
			}
		}
	}
	tokens[token_count] = NULL;
	return token_count;
}

/*
	Reads command and tokenize after
 */
void read_command(char *buff, char *tokens[], bool *in_background)
{
	*in_background = false;

	
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);
	if ( (length < 0) && (errno !=EINTR) )
	{
		perror("Unable to read command. Terminating.\n");
		exit(-1); 
	}

	
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}


	
	int token_count = tokenize_command(buff, tokens);
	if (token_count == 0) {
		return;
	}
	
	// Checks if its in background
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		tokens[token_count - 1] = 0;
	}
	
}
