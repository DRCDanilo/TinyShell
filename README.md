# ENSEA in the Shell - TinyShell

This repository contains the development of a micro shell, which is able to launch commands and display information about their execution.

This project was developed by Joel COLASO and Danilo DEL RIO CISNEROS as a practical work for the Computer Science major module in the second year of the engineering school ENSEA.

## How to Compile

The *Makefile* file developed by the professor Mr. C. BARÈS is needed to compile the project.

Typing the command `make`, it is possible to compile the project.

## How to Execute

To execute the project just type `./TinyShell`

![imagen](https://github.com/user-attachments/assets/1f448950-b79f-4f90-b753-2e38ce614192)

## Running

Typing `./TinyShell`, it is possible to execute the micro shell project:

![image](https://github.com/user-attachments/assets/09bdd14f-1d79-44b5-90d1-f4cce26924bf)

After executing the program, it is possible to launch a command and see the return code or signal, and the time used to executed:

![image](https://github.com/user-attachments/assets/b00726aa-aef6-4d13-9bf6-75926536405e)

## Closing the shell

The shell is exited by simply typing "exit" and pressing enter.

## Commands with arguments

Our shell supports the use of arguments, which must be separated by spaces.

## Redirection of input and output

By typing "<" or/and ">" followed by the path of a file, the input or/and output can be redirected. If the output file doesn't exist, it will be created. If it exists, it will be overwritten.
