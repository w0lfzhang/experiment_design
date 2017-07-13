#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>

#define MAXARGS 20
#define ARGLEN 100

char command[100];
char *args[10];

int parse_args(char *s)
{
	int i = 0, j;
    char *p = strtok(s, " ");
    while(p != NULL)
    {
    	args[i] = (char *)malloc(20);
        strcpy(args[i], p);
        //printf("%d %s\n", i, args[i]);
        i++;
        p = strtok(NULL, " ");
    }
    args[i] = NULL;
    return i + 1;
}

int main()
{
	int pid, count, fd;
	while( 1 )
	{
		fgets(command, 100, stdin);
		command[strlen(command)-1] = '\0';
		count = parse_args(command) - 1;
		//printf("%d\n", count);

		if( strcmp(args[0], "exit") == 0 )
		{
			exit(0);
		}
		if( strcmp(args[0], "cd") == 0 )
		{
			chdir(args[1]);
		}

		if( (pid = fork()) == 0 )
		{
			if( strcmp(args[count-2], ">") == 0 )
			{
				fd = open(args[count-1], O_WRONLY | O_CREAT | O_TRUNC);
				dup2(fd, 1);
				close(fd);   //differrent process
				args[count-2] = NULL;
			}
			//puts("in child process");
			if ( execvp(args[0], args) == -1 )
			{
				perror("fork error!");
			}
		}
		if( pid > 0 )
		{
			wait(NULL);
			//printf("fork done, wait child process to exit!\n");
		}
		if( pid < 0 )
		{
			perror("fork error!");
			exit(0);
		}
		//fflush(stdin);
	}

	return 0;
}