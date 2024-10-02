# Smallsh 
**smallsh** is a simple command-line shell written in C. It supports a number of built-in commands such as `cd`, `exit`, and handles environment variable expansion, input/output redirection, and background processes.

## Features
- Runs system commands such as `ls`, `cd`, etc., using execvp().
- Supports < to redirect input from a file.
- Supports > and >> to redirect output to a file (overwrite and append modes).
- Commands ending with & will run in the background.
- Expands $$ to the process ID (PID) of the shell, and $? to the exit status of the last foreground process.

## Usage
### Starting the Shell
To start the shell, compile the code and run the smallsh executable:

`./smallsh`

### Built-in Commands

`echo Hello World!`
`exit 0`
`cd /tmp`

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

## Test Cases Covered:
- Echo Command
`echo Hello, World!`
> Expected Output: Hello, World!
- Show Current Directory
`pwd`
> Expected Output: current working directory path
- Change Directory
`cd /tmp && pwd`
> Expected Output: /tmp
- Background Process
`sleep 5 &`
> Expected Output: Command runs in the background.
- Input Redirection
`cat < input.txt`
> Expected Output: contents of input.txt.
- Output Redirection
`echo "Hello, redirected!" > output.txt`
> Expected Output: output is written to output.txt
- Append Output
`echo "Appended text" >> output.txt`
Expected Output: text is appended to output.txt
- PID Expansion
`echo $$`
> Expected Output: the PID of the current shell.
-Exit Status
`exit 42`
> Expected Output: Shell exits with status 42.
