/*
project: 01
author: Logan Seely
email: lz50375@umbc.edu
student id: LZ50375
description: a simple linux shell designed to perform basic linux commands
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include "utils.h"

//DEFINE THE FUNCTION PROTOTYPES
void user_prompt_loop();
int get_user_command();
int parse_command(char*, char***, size_t*);
int execute_command(char**, size_t, char*);

int main(int argc, char **argv)
{
  /*
    Write the main function that checks the number of argument passed to ensure 
    no command-line arguments are passed; if the number of argument is greater 
    than 1 then exit the shell with a message to stderr and return value of 1
    otherwise call the user_prompt_loop() function to get user input repeatedly 
    until the user enters the "exit" command.
  */

  // validate number of args
  if (argc > 1)
  {
    perror("Error: too many arguments passed to shell.");
    return 1;
  }

  // enter command loop
  user_prompt_loop();
  
  return 0;
}

/*
  user_prompt_loop():
  Get the user input using a loop until the user exits, prompting the user for a command.
  Gets command and sends it to a parser, then compares the first element to the two
  different commands ("/proc", and "exit"). If it's none of the commands, 
  send it to the execute_command() function. If the user decides to exit, then exit 0 or exit 
  with the user given value. 
*/
void user_prompt_loop()
{
  // initialize variables
  const char prompt[] = "$ ";
  const char EXIT[] = "exit";
  const char PROC[] = "/proc";
  int EXIT_FLAG = -1, RUNNING = 1;

  size_t input_len = sizeof(char)*64; // tracks size of input buffer
  char *input = (char*)calloc(input_len, sizeof(char)); // input buffer
  char **output = (char**)malloc(sizeof(char*)); // output array
  int parsed_args = 0; // number of args parsed by parse_command()
  size_t output_len = 0; // track size of output array
  output[0] = NULL; // initialize output array
  
  do {
    fputs(prompt, stdout);
    int size = get_user_command(&input, &input_len);
    // exit on EOF
    if (size == -1)
    {
      fflush(stdin);
      break;
    }
    
    parsed_args = parse_command(input, &output, &output_len);
    if (parsed_args < 1)
      continue;
    
    // handle proc command
    if (strcmp(output[0], PROC) == 0)
    {
      if (parsed_args != 2)
      {
	continue;
      }
      // use a temp char buffer since strtok modifies its first arg
      char *copy = (char*) malloc(sizeof(char)*(strlen(output[1]) + 2));
      copy = (char*)memcpy(copy, output[1], strlen(output[1]) + 1);

      const char *delim = "/";
      char *type = strtok(copy, delim);
      if (type != NULL)
      {
	// len of path is strlen of the two args plus 3 for the two null terminators
	// and space for a '/' if there wasn't one in the second arg
	size_t path_len = strlen(output[0]) + strlen(output[1]) + 3;

	char *r = (char*)realloc(output[0], path_len);
	while (r == NULL)
	  r = (char*)realloc(output[0], path_len);
	output[0] = r;

	if (output[1][0] != '/')
	{
	  output[0] = strcat(output[0], "/");
	}
	// try to open the given proc file
	const char *path = strcat(output[0], output[1]);
	FILE *procfile = fopen(path, "r");
	if (procfile == NULL)
	{
	  perror("Error: invalid proc filename");
	}
	else
	{
	  // print the file to stdout line by line
	  char *buff = NULL;
	  size_t buff_len = 0;
	  int read = getline(&buff, &buff_len, procfile);
	  while (read != -1)
	  {
	    fputs(buff, stdout);
	    read = getline(&buff, &buff_len, procfile);
	  }
	  // cleanup
	  fclose(procfile);
	  if (buff != NULL)
	    free(buff);
	  fputs("\n", stdout);
	}
      }
      free(copy);
    }
    // handle exit command
    else if (strcmp(output[0], EXIT) == 0)
    {
      if (parsed_args == 1)
      {
	EXIT_FLAG = 0;
	RUNNING = 0;
      }
      // check if there is a second arg and if it is a digit 
      else if (parsed_args == 2 && (output[1][0] >= 30 && output[1][0] <= 57))
      {
        EXIT_FLAG = atoi(output[1]);
	RUNNING = 0;
      }
    }
    else
    {
      // execute a command
      int result = execute_command(output, output_len, input);
      if (result == -1)
	perror("Error: command failed");
    }
    
  } while (RUNNING == 1);

  // deallocate memory
  free(input);
  for (size_t i = 0; i < output_len; i++)
  {
    if (output[i] != NULL)
      free(output[i]);
  }
  free(output);

  exit(EXIT_FLAG);
}

/*
  get_user_command():
  Take input of arbitrary size from the user and return to the user_prompt_loop()
*/
int get_user_command(char** input, size_t* len)
{
  return getline(input, len, stdin);
}

/*
  parse_command():
  Given an input string and output, parses the input into output
  and returns the number of parsed args.
  
  Take command grabbed from the user and parse appropriately.
  Example: 
    user input: "ls -la"
    parsed output: ["ls", "-la", NULL]
  Example: 
    user input: "echo     hello                     world  "
    parsed output: ["echo", "hello", "world", NULL]
*/
int parse_command(char* input, char*** output, size_t* output_len)
{
  int quotes = 0; // use to parse quoted args
  int num_args = 0, ii = 0;
  int arg_len = 4, read = 0;
  char *arg = NULL; // argument buffer 
  
  // reuse or allocate new memory to use
  if ((*output)[0] == NULL)
  {
    arg = (*output)[0] = (char*)calloc((size_t)arg_len, sizeof(char));
    *output_len = 1;
  }
  else
  {
    arg = (*output)[0];
  }
  
  // read input string until null terminator is reached
  while (input[ii] != '\0') // 
  {
    // increase size of arg as needed
    if(read + 1 >= arg_len)
    {
      arg_len = arg_len << 1;
      char *r = (char*)realloc(arg, sizeof(char)*arg_len);
      while (r == NULL)
      {
	r = (char*)realloc(arg, sizeof(char)*arg_len);
      }
      arg = (*output)[num_args] = r;
    }

    // read each non-space character
    if (!isspace(input[ii]))
    {
      // handle single and double quotes
      if (input[ii] == '\"' || input[ii] == '\'')
      {
	// set/unset the quotes flag
        if (ii > 0 && input[ii - 1] != '\\')
        {
	  quotes = !quotes;
	}
	// include escaped quotes in the parsed arg
	else
	{
	  arg[read++] = input[ii];
	}
      }
      else
      {
	arg[read++] = input[ii];
      }
    }
    // include whitespace between quotes
    else if (quotes == 1)
    {
      arg[read++] = input[ii];
    }
    // prepare to parse the next arg
    else if (read > 0 && !isspace(input[ii + 1]))
    {
      // null terminate the currently parsed arg
      arg[read] = '\0';
      read = 0;
      num_args++;

      // reuse memory or allocate new memory to store the next arg
      if ((size_t)(num_args + 1) > *output_len)
      {
	*output_len = num_args + 1;
	char** r = (char**)realloc(*output, sizeof(char*)*(*output_len));
	while (r == NULL)
	{
	  r = (char**)realloc(*output, sizeof(char*)*(*output_len));
	}
	*output = r;
	
	(*output)[num_args] = NULL;
      }

      // only allocate new memory if the input isn't about to end
      if (input[ii + 1] != '\0')
      {
	if ((*output)[num_args] == NULL)
	{
	  (*output)[num_args] = (char*)calloc((size_t)arg_len, sizeof(char));
	}
	arg = (*output)[num_args];
      }
      else
      {
	// set the "last" arg to be a null pointer
	if ((*output)[num_args] != NULL)
	{
	  free((*output)[num_args]);
	}
	(*output)[num_args] = NULL;
      }      
    }
    ii++;
  }

  // unescape each parsed arg, replacing any old args if unescape succeeded
  for (int jj = 0; jj < num_args; jj++)
  {
    char *u = unescape((*output)[jj], stderr);
    if (u != NULL)
    {
      free((*output)[jj]);
      (*output)[jj] = u;
    }
  }
  
  return num_args;
}

/*
  execute_command():
  Execute the parsed command if the commands are neither /proc nor exit;
  fork a process and execute the parsed command inside the child process.
  
  returns -1 on command fail to execute, 0 on success
*/
int execute_command(char **args, size_t args_len, char *input)
{
  int RESULT = 0;
  pid_t pid = 0;
  pid = fork();
  
  // we are in the child: try to execute the command
  if (pid == 0)
  {
    RESULT = execvp(args[0], args);

    // free memory in child process
    for (size_t i = 0; i < args_len; i++)
    {
      free(args[i]);
    }
    free(args);
    free(input);
    exit(RESULT);
  }
  else if (pid < 0)
  {
    perror("Error: child process could not be created\n");
  }
  else
  {
    const int SIZE = 8;
    int *status = (int*)malloc(sizeof(int)*SIZE);
    waitpid(pid, status, 0);

    // handle child process exit status
    if (WIFEXITED(*status))
    {
      RESULT = 0;
    }
    else
    {
      RESULT = -1;
    }
    free(status);
  }
  return RESULT;
}
