/* part1.c
 *
 * Author: Justin Spidell
 *
 * Part 1: MCP Launches the Workload
 *		This version of MCP takes input from a file and launches seperate processes
 *		to run them all.
 *
 * Usage: ./part1 -f <input.txt>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "string_parser.h"

int main(int argc, char* argv[])
{
	FILE* inFPtr;

	// Check Usage
	if (argc < 3) {
		printf("Usage: ./part1 -f <input>\n");
		exit(EXIT_FAILURE);
	}

	// Check Flags
	if (!(strcmp(argv[1], "-f"))) {
		inFPtr = fopen(argv[2], "r");
		// Check File exists
		if (inFPtr == NULL) {
			printf("Error: File does not exist\n");
			exit(EXIT_FAILURE);
		}

	} else {
		printf("Error: Unkown Flag\n");
		exit(EXIT_FAILURE);
	}

	// Declare variables
	int status = 0;

	size_t len = 128;
	char* line_buf = malloc(len);

	command_line token_buffer;

	pid_t* pid_ary = malloc(sizeof(pid_t));
	
	int i = 0;

	// Get commands from file, execute them
	while (getline(&line_buf, &len, inFPtr) != -1)
	{
		// Add more space to pid_ary
		if (i != 0)
		{
			pid_t* tmp = realloc(pid_ary, sizeof(pid_t) * (i + 2));
			if (tmp == NULL)
			{
				printf("Error: realloc Failure\n");
				exit(EXIT_FAILURE);
			} else {
				pid_ary = tmp;
			}
		}
		
		// Get tokens
		token_buffer = str_filler(line_buf, " ");
		// Fork
		pid_ary[i] = fork();
		
		// Check for failure
		if (pid_ary[i] < 0)
		{
			printf("Error: fork failure\n");
			free_command_line(&token_buffer);
			free(line_buf);
			fclose(inFPtr);
			free(pid_ary);
			exit(EXIT_FAILURE);	
		}
		
		if (pid_ary[i] == 0)
		{
			printf("Child Process: %d - Calling exec()\n", getpid());
			// Execute command / Check for failure
			if (execvp(token_buffer.command_list[0], token_buffer.command_list) == -1)
			{
				// Failure
				printf("Error: exec() failure in process: %d\n", getpid());		
				free_command_line(&token_buffer);
				free(line_buf);
				fclose(inFPtr);
				free(pid_ary);
				exit(EXIT_FAILURE);
			}
			// Exit normally
			free_command_line(&token_buffer);
			free(line_buf);
			fclose(inFPtr);
			free(pid_ary);
			exit(-1);
		}
		// free small buffer and memset
		free_command_line(&token_buffer);
		memset(&token_buffer, 0, 0);
		i++;
	}

	int size = i++;

	// Wait for processes
	for (int i = 0; i < size; i++)
	{
		waitpid(pid_ary[i], &status, 0);
	}

	// Exit normally
	free(pid_ary);
	free(line_buf);
	fclose(inFPtr);
	exit(EXIT_SUCCESS);
}
