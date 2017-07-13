#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct param
{
	int start;
	int end;
}param;

#define N 10 //the number of thread

void *calc(void *arg);

int main()
{
	pthread_t tids[N];
	double sum = 0, *result;
	int items = 1000, i;
	param *param, params[N];
	int average = items / N;

	for(i = 0; i < N; i++)
	{
		//must allocate struct param because of data sharing
		param = &params[i];
		param->start = (average * i + 1)*2 - 1;
		param->end = ((i+1)*average)*2 -1;
		if( i == N-1)
		{
			param->end = items*2 -1;
		}
		//printf("%d %d\n", param->start, param->end);
		pthread_create(&tids[i], NULL, calc, param); 		
	}

	for(i = 0; i < N; i++)
	{
		pthread_join(tids[i], (void **)&result);
		printf("%f\n", *result);
		sum += *(double *)result;
		free(result);
	}	
	
	printf("pi: %f\n", 4 * sum);
	return 0;
}

void *calc(void *arg)
{
	double *result = (double *)malloc(sizeof(double));
	if( result == NULL)
	{
		puts("malloc failed");
		exit(0);
	}
	int temp, state;
	param *parameter = (param *)arg;
	
	if( ((parameter->start)/2) % 2 )
	{
		state = -1;
	}
	else
	{
		state = 1;	
	}
	
	for(temp = parameter->start; temp <= parameter->end; temp += 2)
	{
		//printf("calc: temp = %d\n", temp);
		*result += 1.0/temp*state;
		state *= -1;	
	}	
	
	return (void *)result;
}	

