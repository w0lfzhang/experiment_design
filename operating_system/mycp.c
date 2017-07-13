#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>   
#include <fcntl.h>
#include <string.h> 

int main(int argc, char * argv[])
{
	char *buffer;
	int count;

	if( argc < 3 )
	{
		printf("usage: %s source_file dest_file\n", argv[0]);
		exit(0);
	}

	int source_fd = open(argv[1], O_RDONLY);
	if( source_fd == -1 )
	{
		perror("Opening source_file failed");
	}
	int dest_fd = open(argv[2], O_WRONLY);
	if( dest_fd == -1 )
	{
		perror("Opening dest_file failed");
	}

	buffer = malloc(0x100);
	memset(buffer, 0, 0x100);
	if( buffer == NULL )
	{
		printf("malloc error!\n");
		exit(0);
	}

	while( (count = read(source_fd, buffer, 10)) > 0)
	{
		write(dest_fd, buffer, count);
	}

	free(buffer);
	buffer = NULL;
	
	return 0;
}