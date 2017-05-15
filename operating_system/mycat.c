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

	if( argc < 2 )
	{
		printf("usage: %s filename\n", argv[0]);
		exit(0);
	}

	int fd = open(argv[1], O_RDONLY);
	if( fd == -1 )
	{
		perror("Opening file failed");
	}

	buffer = malloc(0x100);
	memset(buffer, 0, 0x100);
	if( buffer == NULL )
	{
		printf("malloc error!\n");
		exit(0);
	}

	/*using lseek to ajust offset? no need to do that*/
	while( (count = read(fd, buffer, 10)) > 0 )  
	{
		printf("%d ", count);
		write(1, buffer, 10);
	}
	
	printf("\n");
	return 0;
}