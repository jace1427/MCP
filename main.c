#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void signaler(pid_t* pid_ary, int size, int sig)
{
	for (int i = 0; i < size; i++)
	{
		printf("Parent process: %d - Sending signal: %d to child process: %d\n", getppid(), sig, getpid());
		kill(pid_ary[i], sig);
	}
}

int main()
{
	int size = 5;

	pid_t* pid_ary = malloc(sizeof(pid_t) * size);

	const char* file = "./iobound";
	char *const argv_exe[4] = {"./iobound", "-seconds", "5", NULL};

	int status = 0;
	pid_t wpid;

	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigprocmask(SIG_BLOCK, &set, NULL);
	int sig;	

	pid_t parent = getpid();

	for (int i = 0; i < size; i++)
	{
		if ((pid_ary[i] = fork()) == 0)
		{
			printf("Child Process: %d - Waiting for SIGUSR1...\n", getpid());
			sigwait(&set, &sig);
			printf("Child Process: %d - Received signal: SIGUSR1 - Calling exec().\n", getpid());
			execvp(file, argv_exe);
		}
	}

	

	if (getpid() == parent)
	{
		sleep(3);
		signaler(pid_ary, 5, SIGUSR1);
		sleep(3);
		signaler(pid_ary, 5, SIGSTOP);
		sleep(5);
		signaler(pid_ary, 5, SIGCONT);
		sleep(3);	
		signaler(pid_ary, 5, SIGINT);
	}

	for (int i = 0; i < size; i++)
	{
		waitpid(pid_ary[i], &status, 0);
	}	
	//while ((wpid = wait(&status)) > 0);

	free(pid_ary);

	return 0;
}

