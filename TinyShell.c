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
#define WAITING_FOR_COMMAND_PROMPT_LENGTH 20
#define COMMAND_BUFFER_SIZE 1000

//Beginning of the code

int main() {

	// FD = File Descriptor

	char greetingMessage[GREETING_MESSAGE_LENGTH] = "Good Morning, welcome to the tiny shell.\n\rType 'exit' to leave.\n\renseash % "; 
	
	int ret = write(STDOUT_FILENO, greetingMessage, GREETING_MESSAGE_LENGTH*sizeof(char));
	
	if(ret == -1) {
		perror("write");
		exit(EXIT_FAILURE);
	}
	
	char commandBuffer[COMMAND_BUFFER_SIZE];
	int commandBufferIndex = 0;
	char inputCharacter;

	while(1) {
		int readReturnValue = read(STDIN_FILENO, &inputCharacter, 1);
		
		if (readReturnValue == -1) {
			// TODO ADD ERROR MANAGEMENT
		} else if (readReturnValue == 0) {
			continue;
		} else if (readReturnValue >= 1) {
			if (inputCharacter == '\n') {
				
				// Complete the command by adding \n at its end
				commandBuffer[commandBufferIndex] = '\0';

				// Execution of the command
				int pid, status;
				pid = fork();
				if(pid != 0) { // father code (waits for child)
					wait(&status) ;
				} else {       // child code (executes the command)
					execlp(commandBuffer, commandBuffer , (char *)NULL) ;
					exit(EXIT_FAILURE);
				}


				// Reinitialising the command buffer
				commandBufferIndex = 0;
				
				// Re-printing the command prompt "enseash % "
				char waitingForCommandPrompt[WAITING_FOR_COMMAND_PROMPT_LENGTH] = "\n\renseash % "; 
				int ret = write(STDOUT_FILENO, waitingForCommandPrompt, WAITING_FOR_COMMAND_PROMPT_LENGTH*sizeof(char));
				if(ret == -1) {
					perror("write");
					exit(EXIT_FAILURE);
				}
				
			} else {
				commandBuffer[commandBufferIndex] = inputCharacter;
				commandBufferIndex++;
				
			}
		}
		
		
		
		//strcmp(inputBuffer, "exit"); //for later
		

	}
}
