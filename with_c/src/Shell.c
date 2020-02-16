#include "Shell.h"
 
// Sigchld Event
void sigchld_handler(int sig){
	if(quitCommand == false){ // Check if quit command cought
		int status;
		pid_t kidpid;
		while((kidpid = waitpid(-1, &status, WNOHANG)) > 0)
		{
			int exitstat = WEXITSTATUS(status);
			printf("\n[%d] retval: %d", kidpid, exitstat); // [pid] retval: exitStatus
			fflush(stdout);
			bgProcessCount--;
		}
	}
	
}
// Checks file existince for redirection
bool isFileExist(char* fileName){
	int file = open(fileName, O_RDONLY);
	if(file > 0 ){
		close(file);
		return true;	
	}
	else return false;
}
void shell(){

	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];
	char execvpError[] = "Unknown Command.\n";
	char invalidDir[] = "Unknown Directory.\n";
	char currentPath[COMMAND_LENGTH];
	// Defines sigchld event for background process 
	struct sigaction sa;
	sa.sa_handler = &sigchld_handler;
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	sigaction(SIGCHLD, &sa, 0);
	
	while (true) 
	{
		int status = 0;
		int error = 0;
		pid_t pid;
		_Bool in_background = false;
		
		getcwd(currentPath, sizeof(currentPath));
		printf("%s> ",currentPath);
		fflush(stdout);
		
		// Waits for command
		read_command(input_buffer, tokens, &in_background);
		
		bool redirIn = false;
		bool redirOut = false;
		char *file_name;
		// Checks tokens for redirection
		for(int i = 0; tokens[i] != NULL; i++){
			if(strcmp(tokens[i], "<") == 0){
				redirIn = true;
				file_name = tokens[i+1];
				tokens[i] = 0;
				tokens[i+1] = 0;		
			}
			else if(strcmp(tokens[i], ">") == 0){
				redirOut = true;
				file_name = tokens[i+1];
				tokens[i] = 0;
				tokens[i+1] = 0;
			}	
		}
		// Checks if redirected file is exist
		if((redirOut == true || redirIn == true) && !isFileExist(file_name)){
			printf("%s",invalidDir);
			continue;		
		}
		
		if(tokens[0] != NULL) 
		{
			if (strcmp(tokens[0], "pwd") == 0)
			{
				getcwd(currentPath, sizeof(currentPath));
				printf("%s",currentPath);
				status = 1;	
			}
			else if (strcmp(tokens[0], "cd") == 0)
			{
				int checkDir = chdir(tokens[1]);
				if (checkDir == -1) 
				{
					printf("%s",invalidDir);
				}
				status = 1;	
			}
			else if (strcmp(tokens[0], "quit") == 0)
			{	
				// Wait until all background process done
				quitCommand = true;
				
				while(bgProcessCount > 0){ 
					int status;
					pid_t kidpid;
					while((kidpid = waitpid(-1, &status, WNOHANG)) > 0)
					{
						int exitStatus = WEXITSTATUS(status);
						printf("\n[%d] retval: %d", kidpid, exitStatus);
						fflush(stdout);
						bgProcessCount--;
					}
				}
								
				exit(0);
			}
			
			
		}
		
		if (status == 0) 
		{
			pid = fork();

			if (pid < 0)
			{
				// if pid lower then 0 means something gone wrong
				perror("Child couldnt created");
			} 

			else if(pid == 0){
				// Sets redirection files
				if(redirIn == true){
					int file = open(file_name, O_RDONLY);
					dup2(file, 0);
					close(file);
				}
				else if(redirOut == true){
					int file = open(file_name, O_WRONLY | O_APPEND);
					dup2(file, 1);
					close(file);
				}
				
				error = execvp(tokens[0], tokens);
				if (error == (-1))
				{
					printf("Error! %s : %s",tokens[0],execvpError);
					exit(0);
				}			
			}
			if(!in_background) 
			{
				
				waitpid(pid, NULL, 0);
			}
			else{
				bgProcessCount++;
			}
			
		}
		if (in_background) 
		{
			printf("[%d] process works in background \n",pid);
		}
		
	}
}
