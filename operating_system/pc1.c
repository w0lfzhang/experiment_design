#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

char buffer1[4], buffer2[4];
int count1 = 0, count2 = 0;
pthread_mutex_t mutex;
pthread_cond_t wait_buffer1_empty;
pthread_cond_t wait_buffer2_empty;
pthread_cond_t wait_buffer1_full;
pthread_cond_t wait_buffer2_full;

int buffer1_is_empty();
int buffer1_is_full();
int buffer2_is_empty();
int buffer2_is_full();
char get_buffer1();
char get_buffer2();
void put_buffer1(char);
void put_buffer2(char);

//start_routine
void *consume(void *arg);
void *compute(void *arg);
void *produce(void *arg);

int main()
{
	pthread_t tid[3];
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&wait_buffer1_full, NULL);
	pthread_cond_init(&wait_buffer1_empty, NULL);
	pthread_cond_init(&wait_buffer2_full, NULL);
	pthread_cond_init(&wait_buffer2_empty, NULL);
	pthread_create(&tid[0], NULL, produce, NULL);
	pthread_create(&tid[1], NULL, compute, NULL);
	pthread_create(&tid[2], NULL, consume, NULL);
	pthread_join(tid[0], NULL);
	pthread_join(tid[1], NULL);
	return 0;
}

int buffer1_is_empty()
{
	if(count1 == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int buffer2_is_empty()
{
	if(count2 == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }	
}

int buffer1_is_full()
{
	if( count1 == 4)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int buffer2_is_full()
{
    if( count2 == 4)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

char get_buffer1()
{
	int i;
	char c = buffer1[0];
	count1 -= 1;
	if(count1 == 0)
	{
		buffer1[0] = '\0';
	}
	else
	{
		for(i = 0; i < count1; i++)
		{
			buffer1[i] = buffer1[i+1];
		}
	}
	return c;
}

char get_buffer2()
{
    int i;
    char c = buffer2[0];
    count2 -= 1;
    if(count2 == 0)
    {
        buffer2[0] = '\0';
    }
    else
    {
        for(i = 0; i < count2; i++)
        {
            buffer2[i] = buffer2[i+1];
        }
    }
    return c;
}

void put_buffer1(char c)
{
	int i;
	buffer1[count1] = c;
	count1 ++;
}

void put_buffer2(char c)
{
    int i;
    buffer2[count2] = c;
    count2 ++;
}

void *produce(void *arg)
{
	int i;
	char c;
	for(i = 0; i < 8; i++)
	{
		pthread_mutex_lock(&mutex);
		while( buffer1_is_full() )
		{
			pthread_cond_wait(&wait_buffer1_empty, &mutex);
		}
		c = i + 'a';
		put_buffer1(c);
		//printf("produce character: %c\n", c);
		sleep(1);
		pthread_cond_signal(&wait_buffer1_full);
		pthread_mutex_unlock(&mutex);
	}	
}

void *compute(void *arg)
{
	int i;
	char c;
	for(i = 0; i < 8; i++)
	{
		pthread_mutex_lock(&mutex);
		while( buffer1_is_empty() )
		{
			pthread_cond_wait(&wait_buffer1_full, &mutex);
		}
		while( buffer2_is_full() )
		{
			pthread_cond_wait(&wait_buffer2_empty, &mutex);
		}
		c = get_buffer1();
		c = c - 32;
		put_buffer2(c);
		//printf("compute character: %c\n", c);
		sleep(1);
		pthread_cond_signal(&wait_buffer1_empty);
		pthread_cond_signal(&wait_buffer2_full);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

void *consume(void *arg)
{
	int i;
	char c;
	for(i = 0; i < 8; i++)
	{
		pthread_mutex_lock(&mutex);
		while( buffer2_is_empty() )
		{
			pthread_cond_wait(&wait_buffer2_full, &mutex);
		}
		c = get_buffer2();
		printf("after computing character: %c\n", c);
		sleep(1);
		pthread_cond_signal(&wait_buffer2_empty);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}
