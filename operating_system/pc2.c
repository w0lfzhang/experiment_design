#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

//#define N 4
#define COUNT N * 2

typedef struct sema
{
	int value;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
}sema_t;

int in1 = 0, in2 = 0;
int out1 = 0, out2 = 0;
char buffer1[4], buffer2[4];

sema_t mutex_1, mutex_2;
sema_t empty_1, empty_2;
sema_t full_1, full_2;

void sema_init(sema_t *sema, int value)
{
	sema->value = value;
	pthread_mutex_init(&sema->mutex, NULL);
	pthread_cond_init(&sema->cond, NULL);
}

void sema_wait(sema_t *sema)
{
	pthread_mutex_lock(&sema->mutex);
	sema->value--;
	while( sema->value < 0 )
	{
		pthread_cond_wait(&sema->cond, &sema->mutex);
	}
	pthread_mutex_unlock(&sema->mutex);
}

void sema_signal(sema_t *sema)
{
	pthread_mutex_lock(&sema->mutex);
	sema->value++;
	pthread_cond_signal(&sema->cond);
	pthread_mutex_unlock(&sema->mutex);
}

int buffer1_is_empty()
{
	return in1 == out1;
}

int buffer2_is_empty()
{
	return in2 == out2;
}

int buffer1_is_full()
{
	return (out1 + 1) % 4;
}

int buffer2_is_full()
{
	return (out2 + 1) % 4;
}

char get_buffer1()
{
	char c;
	c = buffer1[out1];
	out1 = (out1 + 1) % 4;
	return c;
}

char get_buffer2()
{
	char c;
	c = buffer2[out2];
	out2 = (out2 + 1) % 4;
	return c;
}

void put_buffer1(char c)
{
	buffer1[in1] = c;
	in1 = (in1 + 1) % 4;
}

void put_buffer2(char c)
{
	buffer2[in2] = c;
	in2 = (in2 + 1) % 4;
}

void *consume(void *arg)
{
	int i;
	char c;
	for(i = 0; i < 8; i++)
	{
		sema_wait(&full_2);
		sema_wait(&mutex_2);
		c = get_buffer2();
		sema_signal(&mutex_2);
		sema_signal(&empty_2);
		printf("after changing: %c\n", c);
	}
	return NULL;
}

void *compute(void *arg)
{
	int i; 
	char c;
	for(i = 0; i < 8; i++)
	{
		sema_wait(&full_1);
		sema_wait(&mutex_1);
		c = get_buffer1();
		sema_wait(&empty_2);
		sema_wait(&mutex_2);
		c = c - 32;
		put_buffer2(c);
		sema_signal(&mutex_2);
		sema_signal(&full_2);
		sema_signal(&mutex_1);
		sema_signal(&empty_1);
	}
	return NULL;
}

void produce()
{
	int i;
	char c;
	for(i = 0; i < 8; i++)
	{
		sema_wait(&empty_1);
		sema_wait(&mutex_1);

		c = i + 'a';
		put_buffer1(c);

		sema_signal(&mutex_1);
		sema_signal(&full_1);
	}
}

int main()
{
	pthread_t tid[1];
	sema_init(&mutex_1, 1);
	sema_init(&empty_1, 3);
	sema_init(&full_1, 0);
	sema_init(&mutex_2, 1);
	sema_init(&empty_2, 3);
	sema_init(&full_2, 0);

	pthread_create(&tid[0], NULL, consume, NULL);
	pthread_create(&tid[1], NULL, compute, NULL);
	produce();
	pthread_join(tid[0], NULL);
	pthread_join(tid[1], NULL);
	return 0;
}