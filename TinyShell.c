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

#define GREETING_MESSAGE_LENGTH 200
#define GREETING_MESSAGE_TEXT "Good Morning, welcome to the tiny shell.\n\rType 'exit' to leave.\n\renseash % "

#define WAITING_FOR_COMMAND_PROMPT_LENGTH 200
#define WAITING_FOR_COMMAND_PROMPT_TEXT "\n\renseash[%s:%d] %% "
#define TERMINATED_BY_SIGNAL_TEXT "sign"
#define EXITED_NORMALLY_TEXT "exit"

#define COMMAND_BUFFER_SIZE 1000


typedef struct timespec timespec;



int main() {
	// Display greeting message
	char greetingMessage[GREETING_MESSAGE_LENGTH] = GREETING_MESSAGE_TEXT; 
	int ret = write(STDOUT_FILENO, greetingMessage, GREETING_MESSAGE_LENGTH*sizeof(char));
	if(ret == -1) {
		perror("write");
		exit(EXIT_FAILURE);
	}
	
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
				
				// Complete the command by adding \0 at its end
				commandBuffer[commandBufferIndex] = '\0';
				
				if (!strcmp(commandBuffer, "exit")) {
					exit(EXIT_SUCCESS);
				}

				// Execution of the command
				int pid, status;

				pid = fork();
				if(pid != 0) { // father code (waits for child)
					wait(&status) ;
				} else {       // child code (executes the command)
					execlp(commandBuffer, commandBuffer , (char *)NULL);
					exit(EXIT_FAILURE);
				}
				
				// Get if the child was terminated by signal or exited 
				// And with which code
				char* exitOrSignalText;
				int exitOrSignalCode;
				
				if (WIFSIGNALED(status)) {
					exitOrSignalText = TERMINATED_BY_SIGNAL_TEXT;
					exitOrSignalCode = WTERMSIG(status);
				} else if (WIFEXITED(status)){
					exitOrSignalText = EXITED_NORMALLY_TEXT;
					exitOrSignalCode = WEXITSTATUS(status);
				}
				
				// Re-printing the command prompt with information about
				// the child's execution and termination
				char waitingForCommandPrompt[WAITING_FOR_COMMAND_PROMPT_LENGTH] = "\n\renseash["; 
				snprintf(waitingForCommandPrompt, WAITING_FOR_COMMAND_PROMPT_LENGTH, WAITING_FOR_COMMAND_PROMPT_TEXT, exitOrSignalText, exitOrSignalCode);

				int ret = write(STDOUT_FILENO, waitingForCommandPrompt, WAITING_FOR_COMMAND_PROMPT_LENGTH*sizeof(char));				
				if(ret == -1) {
					perror("write");
					exit(EXIT_FAILURE);
				}
				
				// Reinitialising the command buffer
				commandBufferIndex = 0;

			} else {
				// If non newline character was read: add to buffer
				commandBuffer[commandBufferIndex] = inputCharacter;
				commandBufferIndex++;
			}
		}		
	}
}
