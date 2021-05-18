/* part3.c
 *
 * Author: Justin Spidell
 *
 * Part 3: MCP Schedules Processes
 *		This version of MCP takes input from a file and launches seperate processes
 *		to run them all. All processes wait for SIGCONT to begin execution. MCP begins
 *		scheduling the processes in a round robin style. 
 *
 * Usage: ./part3 -f <input.txt>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "string_parser.h"

void signaler(pid_t pid, int sig)
{
	// Send signal
	kill(pid, sig);	
}

int main(int argc, char* argv[])
{
	FILE* inFPtr;

	// Check Usage
	if (argc < 3) {
		printf("Usage: ./part3 -f <input>\n");
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
	size_t len = 128;
	char* line_buf = malloc(len);

	command_line token_buffer;

	pid_t* pid_ary = malloc(sizeof(pid_t) * 1);
	
	int i = 0;

	int sig;
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGCONT);
	sigprocmask(SIG_BLOCK, &set, NULL);

	pid_t parent = getpid();


	// Get commands from file, execute them
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
			printf("Child Process: %d - Ready\n", getpid());
			// Wait for SIGCONT
			sigwait(&set, &sig);	
			printf("Child Process: %d - Received signal: SIGCONT - Calling exec()\n", getpid());
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
			free(pid_ary);
			fclose(inFPtr);
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
		// Declare variables
		int status;
		
		// Block SIGALARM
		sigemptyset(&set);
		sigaddset(&set, SIGALRM);
		sigprocmask(SIG_BLOCK, &set, NULL);
		int size = i++;
		i = 0;
		
		// Initalize finished array
		int finished[size];
		for (int j = 0; j < size; j++)
		{
			finished[j] = 0;
		}

		int fin_ctr = 0;
		
		// MCP Scheduler Loop	
		while(1)
		{
			// Send SIGCONT
			printf("Parent process: %d - Sending signal: SIGCONT to child process: %d\n", getppid(), pid_ary[i]);
			signaler(pid_ary[i], SIGCONT);
			
			// Set alarm
			alarm(1);
			// Wait for alarm
			sigwait(&set, &sig);	
			
			// Send SIGSTOP
			printf("Parent process: %d - Sending signal: SIGSTOP to child process: %d\n", getppid(), pid_ary[i]);
			printf("\n");
			signaler(pid_ary[i], SIGSTOP);
		
			// use waitpid to update status
			waitpid(pid_ary[i], &status, WUNTRACED|WCONTINUED);

			// Check if process is finished
			if ((finished[i] == 0) && (WIFEXITED(status)) && !(WIFSTOPPED(status)))
			{
				finished[i] = 1;
				fin_ctr++;
			}

			// Break if all processes have finished
			if (fin_ctr == size)
			{
				break;
			}

			// Scuffed loop so I don't have to implement a linked list
			// Finds the next unfinished process
			while(1)
			{
				i++;
				if (i == size)
				{
					i = 0;
				}
				if (!finished[i])
				{
					break;
				}
			}
		}
	}

	// Exit normally
	free(pid_ary);
	free(line_buf);
	fclose(inFPtr);
	exit(EXIT_SUCCESS);
}
