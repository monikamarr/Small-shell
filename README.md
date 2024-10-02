# Smallsh - A Small Shell Implementation
## Overview
**smallsh** is a simple Unix-like shell implementation written in C. 
It provides basic shell functionalities like executing commands, handling built-in commands (`_echo`, `_exit`, `_cd`), input/output redirection, and background job processing.
The shell mimics some of the functionality of a bash-like shell but with a minimal set of features.

## Features
- Runs system commands such as `ls`, `cat`, etc., using execvp().
- Mimics Built-in Commands
  - `_echo` prints its arguments to standard output.
  - `_exit [status]` exits the shell with the given exit status (defaults to 0).
  - `_cd [directory]` changes the working directory. Defaults to the home directory if no argument is provided.
- Supports < to redirect input from a file.
- Supports > and >> to redirect output to a file (overwrite and append modes).
- Commands ending with & will run in the background.
- Expands $$ to the process ID (PID) of the shell, and $? to the exit status of the last foreground process.

## Usage
### Starting the Shell
To start the shell, compile the code and run the smallsh executable:

`./smallsh`

### Built-in Commands

`_echo Hello World!`
`_exit 0`
`_cd /tmp`

### Input and Output Redirection

* Input Redirection (<)
`cat < inputfile.txt`

* Output Redirection (>)
`ls > outputfile.txt`

* Append Output Redirection (>>)
`echo "Hello" >> logfile.txt`

### Running Commands in the Background
To run a command in the background, append & at the end of the command:
`sleep 10 &`

### Compilation
To compile the program, use the following: command if using cmake:
- if using cmake: `cmake --build .`
- if you are using gcc: `gcc -o smallsh main.c -D_GNU_SOURCE`


### Testing
You can run a test script test_smallsh.sh to check the behavior of the shell for common commands.

* Make the script executable:
`chmod +x test_smallsh.sh`

* Run the script:
`./test_smallsh.sh`
