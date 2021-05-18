/* part3.c
 *
 * Author: Justin Spidell
 *
 * Part 3: MCP Knows All
 *		This version of MCP takes input from a file and launches seperate processes
 *		to run them all. All processes wait for SIGCONT to begin execution. MCP begins
 *		scheduling the processes in a round robin style. Information about the running processes
 *		is displayed on the console.
 * 
 * INFO:
 *		pid: The ID of the process
 *		Comm: The filename of the executable
 *		State: Process state
 *		ppid: Parent ID
 *		utime: Time the process has been in user-mode
 *		stime: Time the process has been in kernel-mode
 *		rchar: Bytes this process has read
 *		wchar: Bytes this process has written
 *		syscr: Read I/O system calls this process has called
 *		syscw: Write I/O system calls this process has called
 *
 * Usage: ./part4 -f <input.txt>
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

void printer(pid_t* pid_ary, int size)
{
	// Declare Headers
	char h1[] = "pid";
	char h2[] = "Comm";
	char h3[] = "State";
	char h4[] = "ppid";
	char h5[] = "utime";
	char h6[] = "stime";
	char h7[] = "rchar";
	char h8[] = "wchar";
	char h9[] = "syscr";
	char h10[] = "syscw";

	// Print Headers
	printf("%-15s%-15s%-15s%-15s%-15s%-15s%-15s%-15s%-15s%-15s\n", h1, h2, h3, h4, h5, h6, h7, h8, h9, h10);
	printf("============================================================================================================================================\n");
	
	// Loop through pid array and print info
	for (int i = 0; i < size; i++)
	{
		// Declare file pointers
		char fname[1000];
		sprintf(fname, "/proc/%d/stat", pid_ary[i]);
		FILE* stat = fopen(fname, "r");	

		sprintf(fname, "/proc/%d/io", pid_ary[i]);
		FILE* io = fopen(fname, "r");
		
		// Declare variables
		char comm[1000];
		char state;
		int ppid;
		unsigned long utime; 
		unsigned long stime;

		int rchar;
		int wchar;
		int syscr;
		int syscw;

		// If file pointers are null, process is terminated, otherwise print info
		if ((stat == NULL) || (io == NULL))
		{
			printf("%d\t\t--------------------------------------------------------TERMINATED--------------------------------------------------------\n", pid_ary[i]);
		} else {
			// From /proc/[pid]/stat
			fscanf(stat, "%*d %s %c %d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", comm, &state, &ppid, &utime, &stime);
			// From /proc/[pid]/io
			fscanf(io, "rchar: %x\nwchar: %x\nsyscr: %x\nsyscw: %x", &rchar, &wchar, &syscr, &syscw);
			
			// Print
			printf("%-15d%-15s%-15c%-15d%-15lu%-15lu%-15x%-15x%-15x%-15x\n", pid_ary[i], comm, state, ppid, utime, stime, rchar, wchar, syscr, syscw);

			fclose(stat);
			fclose(io);
		}		
	}

	// Seperate Console output
	printf("\nConsole output:\n");
	printf("============================================================================================================================================\n");
}

int main(int argc, char* argv[])
{
	FILE* inFPtr;

	// Check Usage
	if (argc < 3) {
		printf("Usage: ./part4 -f <input>\n");
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

	system("clear");

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
			// Wait for SIGCONT
			sigwait(&set, &sig);	
			// Execute command / Check for failure
			if (execvp(token_buffer.command_list[0], token_buffer.command_list) == -1)
			{
				// Failure
				printf("Error: exec() failure\n");
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
			signaler(pid_ary[i], SIGCONT);

			// Call printer
			printer(pid_ary, size);

			// Set alarm
			alarm(1);
			// Wait for alarm
			sigwait(&set, &sig);	

			// Send SIGSTOP
			signaler(pid_ary[i], SIGSTOP);

			// Clear console 
			system("clear");

			/// Use waitpid to update status
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

	// Exit Normally
	free(pid_ary);
	free(line_buf);
	fclose(inFPtr);
	exit(EXIT_SUCCESS);
}
