#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct sema
{
	int value;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
}sema_t;

sema_t semas[N];

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

void *func1(void *arg)
{
    int i = (int *)(arg);
    int item = 1;
    while( !flag ) 
    {
        buf[i+1]=item;
        sema_signal(&semas[i+1]);
        sema_wait(&semas[i]);
        item=buf[i]+1;
    }
}

void *funcn(void *arg)
{
    int i = (int *)(arg);
    int j = (i == N-1? 0:i+1);
    while( !flag )
    {
        sema_wait(&semas[i]);
        buf[j]=buf[i]+1;
        sema_signal(&semas[j]);
    }
}

int main()
{
	pthread_t tid[N];
    int num0=0,number[N];
    int i;
    for(i =0; i < N; i++)
    {
        sema_init(&semas[i],0);
    }
    pthread_create(&thr[0],NULL,func1,(void *)(&num0));
    for(i=1 ; i<N; i++)
    {
        number[i]=i;
        pthread_create(&tid[i],NULL,funcn,(void *)(&number[i]));
    }
    
    for(int i=0;i < N;i++)
    {
        pthread_join(tid[i],NULL);
    }
    return 0;
}