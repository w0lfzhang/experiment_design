#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <pthread.h>

#define true 1

typedef struct router
{
    int destination;  //the destination router
    int distance;  //distance to the destination router
    int flag;  //whether direct to the destination router
}router_t;

router_t router[4];     
/*for dijkstra*/
#define n 4
#define maxsize 1000

void *server(void *);  //as a server to accept connection
void *client(void *);  //as a client to send data

//void dijkstra(int C[][n], int v); 

/*a, b, c, d four routers*/
//the distance matrix
int C[n][n] = {{maxsize, 5, 5, maxsize}, {5, maxsize, 5, maxsize}, {5, 5, maxsize, 3}, {maxsize, maxsize, 3, maxsize}};

//void update_router();

int main(int argc, char *argv[])
{
    pthread_t ser_thread;
    pthread_t cli_thread;
    int res;
    int port = atoi(argv[1]);
    res = pthread_create(&ser_thread, NULL, server, &port);
    if( res != 0 )
    {
    	perror("Server thread creation failed!");
    	exit(-1);
    }
    puts("Server thread created!");

    sleep(5);
    res = pthread_create(&cli_thread, NULL, client, &port);
    if( res != 0 )
    {
    	perror("Client thread creation failed!");
    	exit(-1);
    }
    puts("Client thread created!");

    pthread_join(ser_thread, NULL);
    pthread_join(cli_thread, NULL);
    return 0;
}

/*listening ports: 10000, 10001, 10002, 10003*/
void *server(void *port)
{
    puts("in server thread");
    int serverfd, connfd;
    socklen_t serlen, clilen;
    //int port = 10001;

    router_t *rt;
    struct sockaddr_in ser_address;
    struct sockaddr_in cli_address;
    ser_address.sin_family = AF_INET;
    ser_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    ser_address.sin_port = htons(*(int *)port);

    serlen  = sizeof(ser_address);
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    bind(serverfd, (struct sockaddr *)&ser_address, serlen);
    listen(serverfd, 10);
    printf("Server listening on port %d\n", *(int *)port);
    
    while( true )
    {
    	clilen = sizeof(cli_address);
    	connfd = accept(serverfd, (struct sockaddr *)&cli_address, &clilen);
    	char *data = (char *)malloc(0x200);

        puts("now receviving data from client");
        read(connfd, data, 0x200);
        rt = (router_t *)data;
    	printf("%d %d %d\n", rt->destination, rt->distance, rt->flag);
    	data = NULL;
    	free(data);
    	close(connfd);
    }
    
    return "sucess";
}

void *client(void *port)
{
    printf("in clinet thread\n");
    int sockfd1, sockfd2, sockfd3;
    socklen_t len;
    router_t *rt = (router_t *)malloc(sizeof(router_t));
    int conport[4] = {10000, 10001, 10002, 10003};
    int res;
    struct sockaddr_in ser_address1, ser_address2, ser_address3;

    while( true )
    {
        switch( *(int *)port )
        {
            case 10000:  
            	//the first connecting
                ser_address1.sin_family = AF_INET;
                ser_address1.sin_addr.s_addr = inet_addr("127.0.0.1");
                ser_address1.sin_port = htons(10001);
                sockfd1 = socket(AF_INET, SOCK_STREAM, 0);
                len = sizeof(ser_address1);
                res = connect(sockfd1, (struct sockaddr *)&ser_address1, len);
                if( res != 0 )
                {
                    perror("connected to 10001 failed!");
                    exit(-1);
                }

                rt->destination = 10002;
                rt->distance = 5;
                rt->flag = 1;   //connect directly
                write(sockfd1, rt, sizeof(router_t));
				close(sockfd1);

				//the second connecting
                ser_address2.sin_family = AF_INET;
                ser_address2.sin_addr.s_addr = inet_addr("127.0.0.1");
                ser_address2.sin_port = htons(10002);
                sockfd2 = socket(AF_INET, SOCK_STREAM, 0);
                len = sizeof(ser_address2);
                res = connect(sockfd2, (struct sockaddr *)&ser_address2, len);
                if( res != 0 )
                {
                    perror("connected to 10002 failed!");
                    exit(-1);
                }

                /*sending router data*/

				rt->destination = 10001;
                rt->distance = 5;
                rt->flag = 1;   //connect directly
                write(sockfd2, rt, sizeof(router_t));
                close(sockfd2);

                break;

            case 10001:
                ser_address1.sin_family = AF_INET;
                ser_address1.sin_addr.s_addr = inet_addr("127.0.0.1");
                ser_address1.sin_port = htons(10000);
                sockfd1 = socket(AF_INET, SOCK_STREAM, 0);
                len = sizeof(ser_address1);
                res = connect(sockfd1, (struct sockaddr *)&ser_address1, len);
                if( res != 0 )
                {
                    perror("connected to 10000 failed!");
                    exit(-1);
                }

                rt->destination = 10002;
                rt->distance = 5;
                rt->flag = 1;   //connect directly
                write(sockfd1, rt, sizeof(router_t));
                close(sockfd1);

                ser_address2.sin_family = AF_INET;
                ser_address2.sin_addr.s_addr = inet_addr("127.0.0.1");
                ser_address2.sin_port = htons(10002);
                sockfd2 = socket(AF_INET, SOCK_STREAM, 0);
                len = sizeof(ser_address2);
                res = connect(sockfd2, (struct sockaddr *)&ser_address2, len);
                if( res != 0 )
                {
                    perror("connected to 10003 failed!");
                    exit(-1);
                }

                rt->destination = 10000;
                rt->distance = 5;
                rt->flag = 1;   //connect directly
                write(sockfd2, rt, sizeof(router_t));
                close(sockfd2);

                break;

            case 10002:
                ser_address1.sin_family = AF_INET;
                ser_address1.sin_addr.s_addr = inet_addr("127.0.0.1");
                ser_address1.sin_port = htons(10000);
                sockfd1 = socket(AF_INET, SOCK_STREAM, 0);
                len = sizeof(ser_address1);
                res = connect(sockfd1, (struct sockaddr *)&ser_address1, len);
                if( res != 0 )
                {
                    perror("connected to 10000 failed!");
                    exit(-1);
                }

                rt->destination = 10001;
                rt->distance = 5;
                rt->flag = 1;   //connect directly
                write(sockfd1, rt, sizeof(router_t));

                rt->destination = 10003;
                rt->distance = 3;
                rt->flag = 1;   //connect directly
                write(sockfd1, rt, sizeof(router_t));

                close(sockfd1);

                ser_address2.sin_family = AF_INET;
                ser_address2.sin_addr.s_addr = inet_addr("127.0.0.1");
                ser_address2.sin_port = htons(10001);
                sockfd2 = socket(AF_INET, SOCK_STREAM, 0);
                len = sizeof(ser_address2);
                res = connect(sockfd2, (struct sockaddr *)&ser_address2, len);
                if( res != 0 )
                {
                    perror("connected to 10001 failed!");
                    exit(-1);
                }

                rt->destination = 10000;
                rt->distance = 5;
                rt->flag = 1;   //connect directly
                write(sockfd2, rt, sizeof(router_t));

                rt->destination = 10003;
                rt->distance = 5;
                rt->flag = 1;   //connect directly
                write(sockfd2, rt, sizeof(router_t));

                close(sockfd2);

                ser_address3.sin_family = AF_INET;
                ser_address3.sin_addr.s_addr = inet_addr("127.0.0.1");
                ser_address3.sin_port = htons(10003);
                sockfd3 = socket(AF_INET, SOCK_STREAM, 0);
                len = sizeof(ser_address3);
                res = connect(sockfd3, (struct sockaddr *)&ser_address3, len);
                if( res != 0 )
                {
                    perror("connected to 10003 failed!");
                    exit(-1);
                }

                rt->destination = 10000;
                rt->distance = 5;
                rt->flag = 1;   //connect directly
                write(sockfd3, rt, sizeof(router_t));

                rt->destination = 10001;
                rt->distance = 5;
                rt->flag = 1;   //connect directly
                write(sockfd3, rt, sizeof(router_t));

                close(sockfd3);

                break;

            case 10003:
                ser_address1.sin_family = AF_INET;
                ser_address1.sin_addr.s_addr = inet_addr("127.0.0.1");
                ser_address1.sin_port = htons(10002);
                sockfd1 = socket(AF_INET, SOCK_STREAM, 0);
                len = sizeof(ser_address1);
                res = connect(sockfd1, (struct sockaddr *)&ser_address1, len);
                if( res != 0 )
                {
                    perror("connected to 10002 failed!");
                    exit(-1);
                }


                //write(sockfd1, rt, sizeof(router_t));
                break;

            default: puts("invalid port!\n");
        }
        

        sleep(60);
    }
    return "success";
}
