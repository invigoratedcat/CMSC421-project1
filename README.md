# CMSC 421 Project 1 Spring 2023
Author: Logan Seely  
Email: lz50375@umbc.edu  
Description: This is a simplified linux shell which can perform basic commands. It supports exit, /proc,
and any command that can be found in the $PATH.

First, the shell handles arguments being passed to it by printing an error message if any args are passed
and exiting with code 1 by checking argc in the main function.

Then the shell enters user_prompt_loop, where it enters the command loop:
It prints a prompt for user input ($), grabs said input with get_user_command, parses that using
parse_command, and finally handles what the arguments and command were.

By using const char buffers and strcmp, custom commands such as "/proc" and "exit" are easily implemented.
User input is stored in a char buffer that gets dynamically (re)allocated memory to store functionally
infinite characters, and flags are used to leave the command loop (exit the shell) and then exit with a
given code.

A double char pointer is used to store the args parsed from parse_command (called output), so each arg can
have as much memory as it needs. In parse_command, the input buffer, pointer to the output pointer,
and pointer to the size of the output array are passed. A flag is used to handle quotes, a char buffer is
used for storing each argument, and counters are used to track where in the input and arg buffer the
program is as well as how many args have been parsed. The main idea is to modify and reuse the memory
stored in output directly, so less memory is required to be allocated. Each char in the input buffer is
read in a while loop, where spaces are used to delimit arguments (unless in unescaped quotes), and the null
terminator stops all parsing. This is achieved by determining if the current char is whitespace and the
next isn't. Then a null terminator is added on and memory is allocated for the next arg buffer if the next
char isn't null, else the next string in output is set to be a null pointer (after freeing it if needed).
Once all args are parsed, they are then unescaped using the unescape function, and finally the number of
parsed args is returned.

After each command is parsed, the program attempts execute it using execute_command, where the output,
output_len, and input are passed. The last two args are passed so that memory can be free'd in the child
process, and the first is obviously passed as it contains the command and its args. fork() and a pid_t
variable are used to create a child process, where the pid is checked to know which process the
program is in. The child uses execvp(), handles freeing its memory, and then exits using the return value
of execvp(), as that would indicate whether the process succeeded or not. The parent waits for the child to
finish using waitpid and has the status stored in an integer, where the WIFEXITED macro is then used to
set the RESULT flag, and then the result is returned so the user_command_loop() knows whether it succeeded.
On an invalid command, the shell prints that it could not be found.


/proc was implemented using a temporary char buffer made using memcpy, strtok, and strcat. First, the
second arg from input is checked using strtok with '/' as the delimeter (a temp buffer is used as strtok
modifies its given string). Next, the file path to open is constructed using strcat after reallocating
enough memory in the first arg string, and the proc file is opened after checking its validity. Each line
in the file is printed to stdout, and finally the file is closed, memory is free'd, and a newline char is
printed to stdout to ensure the next prompt displays properly.
