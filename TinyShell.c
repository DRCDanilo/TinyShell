

/* Our goal for this file is to write a simple shell 
to deepen our understanding of systems' computer science */

//Includes
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysmacros.h>
#include <time.h>
#include <string.h>


//Defines

#define NUMBER_OF_NANOSECONDS_IN_ONE_MILLSECOND 1000000
#define NUMBER_OF_MILLSECONDS_IN_ONE_SECOND 1000

#define GREETING_MESSAGE_LENGTH 200
#define GREETING_MESSAGE_TEXT "Good Morning, welcome to the tiny shell.\n\rType 'exit' to leave.\n\renseash % "

#define WAITING_FOR_COMMAND_PROMPT_LENGTH 200
#define WAITING_FOR_COMMAND_PROMPT_TEXT "\n\renseash[%s:%d|%ldms] %% "
#define TERMINATED_BY_SIGNAL_TEXT "sign"
#define EXITED_NORMALLY_TEXT "exit"

#define COMMAND_BUFFER_SIZE 1000

#define MAXIMUM_NUMBER_OF_ARGUMENTS 50
#define MAXIMUM_LENGTH_OF_ARGUMENT 100

typedef struct timespec timespec;


//TODO ADD ERROR MANAGEMENT
char** separateArguments(char* command, int* nArguments) {
	char** arguments = malloc(MAXIMUM_NUMBER_OF_ARGUMENTS * sizeof(char*));

	const char s[2] = " ";
	char *token;
	int iArguments = 1;

	token = strtok(command, s);
	
	while( token != NULL ) {
        arguments[iArguments] = malloc(strlen(token) + 1); 
		strcpy(arguments[iArguments], token);

		token = strtok(NULL, s);
	}
	
	arguments[0] = arguments[1];
	
	*nArguments = iArguments;
	
	return arguments;
}

void freeArguments(char** arguments, int nArguments) {
    for (int i = 1; i <= nArguments; ++i) {
        free(arguments[i]);
    }
    free(arguments);
}



void displayGreeting() {
	char greetingMessage[GREETING_MESSAGE_LENGTH] = GREETING_MESSAGE_TEXT; 
	int ret = write(STDOUT_FILENO, greetingMessage, GREETING_MESSAGE_LENGTH*sizeof(char));
	if(ret == -1) {
		perror("write");
		exit(EXIT_FAILURE);
	}
}

void displayPrompt(int fileDescriptor, char* exitOrSignalText, int exitOrSignalCode, long executionTime) {
	char waitingForCommandPrompt[WAITING_FOR_COMMAND_PROMPT_LENGTH]; 
	snprintf(waitingForCommandPrompt, WAITING_FOR_COMMAND_PROMPT_LENGTH, WAITING_FOR_COMMAND_PROMPT_TEXT, exitOrSignalText, exitOrSignalCode, executionTime);

	int ret = write(fileDescriptor, waitingForCommandPrompt, strlen(waitingForCommandPrompt));				
	if(ret == -1) {
		perror("write");
		exit(EXIT_FAILURE);
	}
}

void executeCommand (char* commandBuffer, char** exitOrSignalText_output, int* exitOrSignalCode_output, long* executionTime_output) {
	
	if (!strcmp(commandBuffer, "exit")) {
		exit(EXIT_SUCCESS);
	}
	
	int nArguments;
	
	char** arguments = separateArguments(commandBuffer, &nArguments);
	
	arguments[nArguments] = (char *)NULL;
	nArguments++;

	int pid, status, ret;
	timespec beforeExec, afterExec;
	
	ret = clock_gettime(CLOCK_REALTIME, &beforeExec);
	if(ret == -1) {
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}
	
	pid = fork();
	if(pid != 0) { // father code (waits for child)
		waitpid(pid , &status , 0) ;
	} else {       // child code (executes the command)
		//execvp(arguments[0], arguments) ;
		execlp(commandBuffer, commandBuffer , (char *)NULL) ;
		exit(EXIT_FAILURE);
	}
	
	ret = clock_gettime(CLOCK_REALTIME, &afterExec);
	if(ret == -1) {
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}		
	
	long executionTime = NUMBER_OF_MILLSECONDS_IN_ONE_SECOND*(afterExec.tv_sec	- beforeExec.tv_sec) + ((afterExec.tv_nsec	- beforeExec.tv_nsec)/NUMBER_OF_NANOSECONDS_IN_ONE_MILLSECOND);	
	
	// Get if the child was terminated by signal or exited 
	// and with which code
	char* exitOrSignalText;
	int exitOrSignalCode;
	
	if (WIFSIGNALED(status)) {
		exitOrSignalText = TERMINATED_BY_SIGNAL_TEXT;
		exitOrSignalCode = WTERMSIG(status);
	} else if (WIFEXITED(status)){
		exitOrSignalText = EXITED_NORMALLY_TEXT;
		exitOrSignalCode = WEXITSTATUS(status);
	}
	
	*exitOrSignalText_output = exitOrSignalText;
	*exitOrSignalCode_output = exitOrSignalCode;
	*executionTime_output    = executionTime;
	
	freeArguments(arguments, nArguments);
}



int main() {
	displayGreeting();
	
	char commandBuffer[COMMAND_BUFFER_SIZE];
	int commandBufferIndex = 0;
	char inputCharacter;

	// Main loop for processing all commands
	while(1) {
		int readReturnValue = read(STDIN_FILENO, &inputCharacter, 1);
		if (readReturnValue == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		} else if (readReturnValue == 0) {
			//If nothing was read, skip to the next iteration
			continue;
		} else if (readReturnValue >= 1) {
			if (inputCharacter == '\n') {
				// This is the case for when the command has ended and 
				// needs to be executed
				
				commandBuffer[commandBufferIndex] = '\0';
		
				char* exitOrSignalText;
				int exitOrSignalCode;
				long executionTime;
		
				executeCommand(commandBuffer, &exitOrSignalText, &exitOrSignalCode, &executionTime);
			
				displayPrompt(STDOUT_FILENO, exitOrSignalText, exitOrSignalCode, executionTime);
				
				commandBufferIndex = 0;

			} else {
				// If non newline character was read: add to buffer
				commandBuffer[commandBufferIndex] = inputCharacter;
				commandBufferIndex++;
			}
		}		
	}
}
