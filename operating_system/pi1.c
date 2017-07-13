#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void *calc(void *arg);

int main()
{
  pthread_t tid;
  double result = 0, *sum;
  int items = 10001, i;
  int state = 1, temp = 1;
  pthread_create(&tid, NULL, calc, (void *)&items);
  
  for( i = 0; i < items/2; i++ )
  {
  	//printf("%d\n", temp);
    result += 1.0 / temp * state;
    temp += 2;
    state *= -1;
  }
  
  pthread_join(tid, (void **)&sum);
  result += *(double *)sum;
  printf("pi: %f\n", 4 * result); 
  return 0;
}

void *calc(void *arg)
{
  static double sum =  0;
  size_t i;
  int end = *(int *)arg;
  int temp, state;
  if( end % 2 )
  {
    state = -1;
    temp = end;
  }
  else
  {
    state = 1;
    temp = end + 1;
  }

  for( i = end/2; i < end; i++)
  {
  	//printf("%d\n", temp);
    sum += 1.0/temp*state;
    state *= -1;
    temp += 2;
  }
  
  return (void *)&sum;
}
