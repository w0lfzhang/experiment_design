#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	while(argc-- > 1)
	{
		printf("%s\t", *++argv);
	}
      
    printf("\n");

	return 0;
}