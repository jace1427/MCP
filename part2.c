/* part2.c
 *
 * Author: Justin Spidell
 *
 * Part 2: MCP Controls the Workload
 *		This version of MCP takes input from a file and launches seperate processes
 *		to run them all. All processes wait for SIGUSR1 to begin execution. MCP then 
 *		sends a SIGSTOP to all processes, waits for 10 seconds, then sends a SIGCONT.
 *
 * Usage: ./part2 -f <input.txt>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "string_parser.h"

void signaler(pid_t* pid_ary, int size, int sig)
{
	// Loop through pid array and send signal
	for (int i = 0; i < size; i++)
	{
		printf("Parent process: %d - Sending signal: %d to child process: %d\n", getppid(), sig, pid_ary[i]);
		kill(pid_ary[i], sig);
	}
}

int main(int argc, char* argv[])
{
	FILE* inFPtr;

	// Check Usage
	if (argc < 3) {
		printf("Usage: ./part2 -f <input>\n");
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

	pid_t* pid_ary = malloc(sizeof(pid_t) * 1);
	
	int i = 0;

	int sig;
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigprocmask(SIG_BLOCK, &set, NULL);

	pid_t parent = getpid();

	// Main While loop
	while (getline(&line_buf, &len, inFPtr) != -1)
	{
		// Add more space to pid_ary
		if (i != 0)
		{
			pid_t* tmp = realloc(pid_ary, sizeof(pid_t) * (i + 2));
			if (tmp == NULL)
			{
				printf("Error: realloc failure\n");
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
			printf("Child Process: %d - Waiting for SIGUSR1...\n", getpid());
			// Wait for SIGUSR1
			sigwait(&set, &sig);	
			printf("Child Process: %d - Received signal: SIGUSR1 - Calling exec()\n", getpid());
			// Execute command / Check for failure
			if (execvp(token_buffer.command_list[0], token_buffer.command_list) == -1)
			{
				// Failure
				printf("Error: execvp failure in process: %d\n", getpid());		
				free_command_line(&token_buffer);
				free(line_buf);
				free(pid_ary);
				fclose(inFPtr);
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

	// Parent Code
	if (getpid() == parent)
	{	
		int size = i++;

		// Wait for processes to start, the send SIGUSR1
		sleep(2);
		printf("\n");
		printf("----------All Processes started, Sending SIGUS1----------\n");
		signaler(pid_ary, size, SIGUSR1);
		sleep(2);

		// Send SIGSTOP
		printf("\n");	
		printf("---------------------Sending SIGSTOP---------------------\n");
		signaler(pid_ary, size, SIGSTOP);
		printf("---------------Stopped, waiting for 10sec----------------\n");
		sleep(10);
		
		// Send SIGCONT
		printf("\n");
		printf("---------------------Sending SIGCONT---------------------\n");
		signaler(pid_ary, size, SIGCONT);
	
		// Wait for all processes to finish
		for (int i = 0; i < size; i++)
		{
			waitpid(pid_ary[i], &status, 0);
		}

	}
	
	// Exit normally
	free(pid_ary);
	free(line_buf);
	fclose(inFPtr);
	exit(EXIT_SUCCESS);
}
