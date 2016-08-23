#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 256
#define TOKEN_NB 10


// This is a minimal C shell that only execute command and arguments
// The purpose of this C programme is to give a basic skeleton to improve
// for people who would not know how to start coding their own shell
// 
// It will only accept up to 8 arguments
//
// IT DOES NOT SUPPORT:
// 	IO REDIRECTION
// 	WILDCARD EXPANSION
//	PIPES
//	BACKGROUND PROCESSES
//	COMMAND SUBSTITUTION


char *backup_addr;
int backup_idx;

// Init the command buffer which is an array of string (char **)
// each string is allocated as buffer of BUFF_SIZE bytes 
void init_cmd_buf(char **cmd)
{
	int i;
	for (i = 0; i < TOKEN_NB; ++i)
	{
		if ((cmd[i] = (char *) malloc(BUFF_SIZE)) == NULL)
		{
			printf("buffer cmd initialization failed - exiting now\n");
			exit(1);
		}
		*cmd[i] = 0;
	}
}

void cleanup(char **cmd)
{
	int i;
	for (i = 0; i < TOKEN_NB; ++i)
	{
		free(cmd[i]);
	}
}

// Tokenize the user input to pass to the shell command by using strktok
// and strcpy to copy the token to the cmd buffer
//
// the user input will "ls -al" will be tokenized as the following
// 	cmd[0] = "ls"
// 	cmd[1] = "-al"
// 	cmd[2] = NULL
void tokenize(char **cmd, char *input)
{
	int i = 0;
	char *token = strtok(input, " \t\n");
	while (token)
	{
		strcpy(cmd[i++], token);
		token = strtok(NULL, " \t\n");	
	}

	// exec calls take expect an array terminated by a null pointer
	backup_addr = cmd[i];
	backup_idx = i;
	cmd[i] = NULL;
}

void restore(char **cmd)
{
	cmd[backup_idx] = backup_addr;
}

// handles the actual shell work
// Can be improved to support additional features
void shell(char **cmd)
{
	pid_t pid = fork();
	
	if (pid < 0)
	{
		printf("fork failed - exiting now");
		exit(1);
	}

	if (pid == 0)
	{
		// child process
#ifdef DEBUG
		printf("DEBUG: child process\n");
#endif
		execvp(cmd[0], cmd);

		// exec does not return on success
		// if we reach this part of the code
		// we need to terminate
		// _exit is used instead of exit as a best practice
		// to avoid messing up with the parent resources
		// by calling atexit handlers, etc..
		printf("execve failed - exiting now");
		_exit(1);
	}
	else
	{
		// parent process
		int status;

		// does not implement background process
		// so we wait for the child process to terminate
		waitpid(pid, &status, 0);		
	}
}

int main(void)
{
	char *input = (char *) malloc(BUFF_SIZE);
	char **cmd = (char **) malloc(10 * sizeof(char *));
	
	size_t sz = BUFF_SIZE;

	if (!input)
	{
		printf("malloc failed - exiting now\n");
		exit(1);	
	}
	
	init_cmd_buf(cmd);
	printf("> ");
	while (getline(&input, &sz, stdin) != -1)
	{
#ifdef DEBUG
		printf("DEBUG: input is %s\n", input);
#endif		
		tokenize(cmd, input);		
		
		shell(cmd);
		restore(cmd);	
		printf("> ");
	}

	cleanup(cmd);
	free(cmd);
	free(input);
	printf("\n");
	return 0;
}
