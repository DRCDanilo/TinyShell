// Simple shell that 

// Included libraries

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

//Constants definitions

#define NUMBER_OF_NANOSECONDS_IN_ONE_MILLSECOND 1000000
#define NUMBER_OF_MILLSECONDS_IN_ONE_SECOND 1000

#define GREETING_MESSAGE_LENGTH 200
#define GREETING_MESSAGE_TEXT "Good Morning, welcome to the tiny shell.\n\rType 'exit' to leave.\n\renseash % "

#define WAITING_FOR_COMMAND_PROMPT_LENGTH 200
#define WAITING_FOR_COMMAND_PROMPT_TEXT "\n\renseash[%s:%d|%ldms] %% "
#define TERMINATED_BY_SIGNAL_TEXT "sign"
#define EXITED_NORMALLY_TEXT "exit"

#define EXIT_SHELL_COMMAND_STRING "exit"

#define COMMAND_BUFFER_SIZE 1000

#define MAXIMUM_NUMBER_OF_ARGUMENTS 50
#define MAXIMUM_LENGTH_OF_ARGUMENT 100

#define FILENAME_MAX_LENGTH 200

#define SPACE_CHARACTER ' '
#define END_OF_STRING_CHARACTER '\0'
#define END_OF_LINE_CHARACTER '\n'

#define SPACE_STRING " "
#define INPUT_REDIRECTION_STRING "<"
#define OUTPUT_REDIRECTION_STRING ">"
#define EMPTY_STRING ""

#define WRITE_ERROR_MESSAGE_STRING "write"
#define READ_ERROR_MESSAGE_STRING "read"




typedef struct timespec timespec;


char* removeInitialTrailingSpaces(char* string) {
	int firstNonSpaceIndex = 0;
	while (string[firstNonSpaceIndex] == SPACE_CHARACTER) {
		firstNonSpaceIndex++;
	}
	
	return &(string[firstNonSpaceIndex]);
}

char** separateArguments(char* command, int* nArguments) {	
	char** arguments = malloc(MAXIMUM_NUMBER_OF_ARGUMENTS * sizeof(char*));

	const char s[2] = SPACE_STRING;
	char *token;

	int iArguments = 0;

	token = strtok(command, s);
	
	while( token != NULL ) {
        arguments[iArguments] = malloc(strlen(token) + 1); 
		strcpy(arguments[iArguments], token);
		iArguments++;
		token = strtok(NULL, s);
	}
		
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
		perror(WRITE_ERROR_MESSAGE_STRING);
		exit(EXIT_FAILURE);
	}
}

void displayPrompt(int fileDescriptor, char* exitOrSignalText, int exitOrSignalCode, long executionTime) {
	char waitingForCommandPrompt[WAITING_FOR_COMMAND_PROMPT_LENGTH]; 
	snprintf(waitingForCommandPrompt, WAITING_FOR_COMMAND_PROMPT_LENGTH, WAITING_FOR_COMMAND_PROMPT_TEXT, exitOrSignalText, exitOrSignalCode, executionTime);

	int ret = write(fileDescriptor, waitingForCommandPrompt, strlen(waitingForCommandPrompt));				
	if(ret == -1) {
		perror(WRITE_ERROR_MESSAGE_STRING);
		exit(EXIT_FAILURE);
	}
}

int findRedirectionInputOrOutput(char* command, char* filenameString, char* delimitationSymbol) {
    // We initially planned to write 2 quasi-identical functions
    // for < (input) and > (output), but we decided to 
    // pass the character as argument to avoid useless repetition
    
    char* beginning = strstr(command, delimitationSymbol);
	
	if(beginning != NULL) {
		
		int j = strlen(delimitationSymbol);

		while (beginning[j] == SPACE_CHARACTER) {
			j++;
		}
		char* ending = strstr(&(beginning[j]), SPACE_STRING);
		if (ending == NULL) {
			strncpy(filenameString, &(beginning[j]), strlen(&(beginning[j])));
			filenameString[strlen(&(beginning[j]))] = END_OF_STRING_CHARACTER;
			strcpy(beginning, EMPTY_STRING);
		} else {
			strncpy(filenameString, &(beginning[j]), (ending - &(beginning[j])));
			filenameString[ending - &(beginning[j])] = END_OF_STRING_CHARACTER;
			strcpy(beginning, ending);
		}
				
		return 1;
	} else {
		return 0;
	}
}

void executeCommand (char* commandBuffer, char** exitOrSignalText_output, int* exitOrSignalCode_output, long* executionTime_output) {
	int ret =  0;
	
	commandBuffer = removeInitialTrailingSpaces(commandBuffer);

	if (!strcmp(commandBuffer, EXIT_SHELL_COMMAND_STRING)) {
		exit(EXIT_SUCCESS);
	}
	
	int stdinBackup = -1;
	int stdoutBackup = -1;
	
	char inputFilenameString[FILENAME_MAX_LENGTH];
	char outputFilenameString[FILENAME_MAX_LENGTH];
	
	if (findRedirectionInputOrOutput(commandBuffer, inputFilenameString, INPUT_REDIRECTION_STRING)){
		int fdInput = open(inputFilenameString, O_RDONLY , S_IRUSR | S_IWUSR);
		if (fdInput == -1) {
			perror("open");
			exit(EXIT_FAILURE);
		}
		stdinBackup = dup(STDIN_FILENO);
		if (stdinBackup == -1) {
			perror("dup");
			exit(EXIT_FAILURE);
		}
		ret = dup2(fdInput, STDIN_FILENO);
		if (ret == -1) {
			perror("dup2");
			exit(EXIT_FAILURE);
		}
        close(fdInput);
	}

	if (findRedirectionInputOrOutput(commandBuffer, outputFilenameString, OUTPUT_REDIRECTION_STRING)){
		int fdOutput = open(outputFilenameString, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		if (fdOutput == -1) {
			perror("open");
			exit(EXIT_FAILURE);
		}
		stdoutBackup = dup(STDOUT_FILENO);
		if (stdoutBackup == -1) {
			perror("dup");
			exit(EXIT_FAILURE);
		}
		ret = dup2(fdOutput, STDOUT_FILENO);
        close(fdOutput);
        if (ret == -1) {
			perror("dup2");
			exit(EXIT_FAILURE);
		}

	}
	
	int nArguments;
	
	char** arguments = separateArguments(commandBuffer, &nArguments);
	
	arguments[nArguments] = (char *)NULL;
	nArguments++;

	int pid, status;
	timespec beforeExec, afterExec;
	
	ret = clock_gettime(CLOCK_REALTIME, &beforeExec);
	if(ret == -1) {
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}
	
	pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	} else if (pid != 0) { // father code (waits for child)
		ret = waitpid(pid , &status , 0) ;
		if (ret == -1) {
			perror("waitpid");
			exit(EXIT_FAILURE);
		}
	} else {               // child code (executes the command)
		execvp(commandBuffer, arguments) ;
		perror("execvp");
		exit(EXIT_FAILURE);
	}
	
	ret = clock_gettime(CLOCK_REALTIME, &afterExec);
	if(ret == -1) {
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}
	
	long executionTime = NUMBER_OF_MILLSECONDS_IN_ONE_SECOND*(afterExec.tv_sec	- beforeExec.tv_sec) + ((afterExec.tv_nsec	- beforeExec.tv_nsec)/NUMBER_OF_NANOSECONDS_IN_ONE_MILLSECOND);	
	
	if (stdinBackup != -1) {
		close(STDIN_FILENO);
		dup2(stdinBackup, STDIN_FILENO);
		close(stdinBackup);
	}
	if (stdoutBackup != -1) {
		close(STDOUT_FILENO);
		dup2(stdoutBackup, STDOUT_FILENO);
		
		close(stdoutBackup);
	}
	
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

	while(1) {
		int readReturnValue = read(STDIN_FILENO, &inputCharacter, 1);
		if (readReturnValue == -1) {
			perror(READ_ERROR_MESSAGE_STRING);
			exit(EXIT_FAILURE);
		} else if (readReturnValue == 0) {
			//If nothing was read, skip to the next iteration
			continue;
		} else if (readReturnValue >= 1) {
			if (inputCharacter == END_OF_LINE_CHARACTER) {
				// Command has ended and needs to be executed
				
				commandBuffer[commandBufferIndex] = END_OF_STRING_CHARACTER;
		
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
