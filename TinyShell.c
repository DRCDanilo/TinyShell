/* Our goal for this file is to write a simple shell 
to deepen our understanding of systems' computer science */

//Includes
#include <sys/types.h>
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

//Beginning of the code

int main() {

	char greetingMessage[GREETING_MESSAGE_LENGTH] = "Good Morning, welcome to the tiny shell.\n\rType 'exit' to leave.\n\renseash % "; 
	
	int ret = write(STDOUT_FILENO, greetingMessage, GREETING_MESSAGE_LENGTH*sizeof(char));
	
	if(ret == -1) {
		perror("write");
		exit(EXIT_FAILURE);
	}
	
}
