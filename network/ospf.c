/*
author: w0lfzhang
date: 2017.04
env: ubuntu 14.04
gcc -o ospf ospf.c `pkg-config --cflags --libs gtk+-2.0`
*/

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
#include <gtk/gtk.h>

#define true 1

typedef struct router
{
    char source;  
    char neighbor;  //the neighbor router
    int distance;  //distance to the neighbor router
}router_t;


/*for dijkstra*/
#define V_SIZE 4
#define NODE 3
#define inf 1000
/*a, b, c, d four routers*/
/*the distance matrix*/
static router_t *router;
static int C[V_SIZE][V_SIZE];
static unsigned id_port;
static char *min_path[NODE];
static int flag = 0;

void *server(void *);  //as a server to accept connection
void *client(void *);  //as a client to send data
//void *show(void *);

int make_addr(struct sockaddr_in *ser_address, int port);
void dijkstra(int graph[][V_SIZE], int n, int start); 
void init_router(router_t *rt, int port);
int update_router(router_t *rt1, router_t * rt2);
void generate_graph_array(router_t *rt, int target[][V_SIZE]);
int get_count(router_t *rt);
void array_fill(int * array, int len, int val);
void close_app(GtkWidget *window, gpointer data);
gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data);


int main(int argc, char *argv[])
{
    int j;
    for(j = 0; j < NODE; j++)
    {
	    min_path[j] = (char*)malloc(0x20);
    }
    pthread_t ser_thread;
    pthread_t cli_thread;
   
    int res;
    int port = atoi(argv[1]);
    id_port = port;
    res = pthread_create(&ser_thread, NULL, server, &port);
    if( res != 0 )
    {
    	perror("Server thread creation failed!");
    	exit(-1);
    }
    //puts("Server thread created!");

    sleep(5);
    res = pthread_create(&cli_thread, NULL, client, &port);
    if( res != 0 )
    {
    	perror("Client thread creation failed!");
    	exit(-1);
    }
    //puts("Client thread created!");
    
    //show the path//
    //*********************//

    sleep(15);

    char content[30];
    if (flag == 1)
    {
        GtkWidget *window;
        GtkWidget *label0, *label1, *label2, *label3;
        GtkWidget *hbox;
        GtkWidget *vbox;

        gtk_init(&argc, &argv);
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

        gtk_window_set_title(GTK_WINDOW(window), "OSPF PROJECT");
        gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_NONE);
        gtk_window_set_default_size(GTK_WINDOW(window), 500, 400);

        g_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(close_app), NULL);
        g_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(delete_event), NULL);


        label0 = gtk_label_new("The shortest path");

		strcpy(content, "the first:  ");
        strcat(content, min_path[0]);
        label1 = gtk_label_new(content);

		strcpy(content, "the second: ");
        strcat(content, min_path[1]);
        label2 = gtk_label_new(content);

		strcpy(content, "the thrid:  ");
        strcat(content, min_path[2]);
        label3 = gtk_label_new(content);

        hbox = gtk_hbox_new(TRUE, 5);
        vbox = gtk_vbox_new(TRUE, 10);

        gtk_box_pack_start(GTK_BOX(vbox), label0, TRUE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(vbox), label1, TRUE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(vbox), label2, TRUE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(vbox), label3, TRUE, FALSE, 5);

        gtk_container_add(GTK_CONTAINER(window), vbox);
        gtk_widget_show_all(window);
        gtk_main();
    }

    
    //*********************//

    pthread_join(ser_thread, NULL);
    pthread_join(cli_thread, NULL);

    /*release the source*/
    int i;
    for(i = 0; i < NODE; i++)
    {
        min_path[i] = NULL;
        free(min_path[i]);
    }

    router = NULL;
    free(router);
    return 0;
}

void close_app(GtkWidget *window, gpointer data)
{
    gtk_main_quit();
}

gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    printf("In delete event\n");
    return FALSE;
}

void array_fill(int * array, int len, int val)
{
    int i;
    for (i = 0; i < len; i++) 
    {
        array[i] = val;
    }
}

void dijkstra(int graph[][V_SIZE], int n, int start)
{
    int dist[n];
    int *path = (int*)malloc(sizeof(int) * n);
    int *shortest = (int*)malloc(sizeof(int) * n);
    int *mark = (int*)malloc(sizeof(int) * n);
    int min, v, i, j;
    array_fill(mark, n, 0);
    array_fill(dist, n, inf);

    for(i = 0; i < n; i++)
    {
        dist[i] = graph[start][i];
        if(i != start && dist[i] < inf)
        {
            path[i] = start;
        }
        else 
        {
            path[i] = -1;
        }

    }

    mark[start] = 1;
    while( true )
    {
        min = inf;
        v = -1;
        //find the min distance
        for(i = 0; i < n; i++)
        {
            if( !mark[i] )
            {
                if(dist[i] < min)
                {
                    min = dist[i];
                    v = i;
                }
            }
        }

        if(v == -1)
        {
            break;
        } 
        //update the path
        mark[v] = 1;
        for(i = 0; i < n; i++)
        {
            if( !mark[i] &&
                graph[v][i] != inf &&
                dist[v] + graph[v][i] < dist[i] )
            {
                dist[i] = dist[v] + graph[v][i];
                path[i] = v;
            }
        }
    }

    //print the path
    int i_path = 0;
    printf("source\tdestination\tmin_distacne\tpath \n");
    for(i = 0; i < n; i++)
    {
        if( i == start) 
        {
            continue;
        }
        array_fill(shortest, n, 0);
        printf("%d\t", start);
        printf("%d\t\t", i);
        printf("%d\t\t", dist[i]);
        int k = 0;
        shortest[k] = i;
        while( path[shortest[k]] != start )
        {
            k++;
            shortest[k] = path[shortest[k-1]];
        }
        k++;
        shortest[k] = start;
        
        memset(min_path[i_path], 0, 0x20);
        char des[2] = {0, 0};
        for(j = k; j > 0; j--)
        {
            printf("%d->", shortest[j]);
            sprintf(des, "%d", shortest[j]);
            strcat(min_path[i_path], des);
            strcat(min_path[i_path], "->");
        }
        printf("%d\n", shortest[0]);
        sprintf(des, "%d", shortest[j]);
        strcat(min_path[i_path], des);
        //puts(min_path[i_path]);
        i_path += 1;
    }

    path = NULL;
    shortest = NULL;
    mark = NULL;
    free(path);
    free(shortest);
    free(mark);
    return ;
}

void generate_graph_array(router_t *rt, int target[][V_SIZE])
{
    //int target[V_SIZE][V_SIZE] = {{inf, 5, 5, inf}, {5, inf, 5, inf}, {5, 5, inf, 3}, {inf, inf, 3, inf}};

	flag = 1;
    //initialize the graph array
    size_t i, j;
    unsigned int count = get_count(rt);
    for(i = 0; i < V_SIZE; i++)
    {
        for(j = 0; j < V_SIZE; j++)
        {
            target[i][j] = inf;
        }
    }

    for(i = 0; i < count; i++)
    {
        if( rt[i].source == 'a' )
        {
            if( rt[i].neighbor == 'b' )
            {
                target[0][1] = rt[i].distance;
            }
            else//( rt[i].neighbor == 'c')
            {
                target[0][2] = rt[i].distance;
            }
        }

        if( rt[i].source == 'b' )
        {
            if( rt[i].neighbor == 'a' )
            {
                target[1][0] = rt[i].distance;
            }
            else//( rt[i].neighbor == 'c')
            {
                target[1][2] = rt[i].distance;
            }
        }

        if( rt[i].source == 'c' )
        {
            if( rt[i].neighbor == 'a' )
            {
                target[2][0] = rt[i].distance;
            }
            else if( rt[i].neighbor == 'b' )
            {
                target[2][1] = rt[i].distance;
            }
            else
            {
                target[2][3] = rt[i].distance;
            }
        }

        if( rt[i].source == 'd' )
        {
            target[3][2] = rt[i].distance;
        }
    }

    switch( id_port )
    {
        case 10000:
            dijkstra(target, V_SIZE, 0);
            break;
        case 10001:
            dijkstra(target, V_SIZE, 1);
            break;
        case 10002:
            dijkstra(target, V_SIZE, 2);
            break;
        case 10003:
            dijkstra(target, V_SIZE, 3);
            break;
        default:
            puts("No such port");
    }
    
}

int make_addr(struct sockaddr_in *ser_address, int port)
{
	ser_address->sin_family = AF_INET;
    ser_address->sin_addr.s_addr = inet_addr("127.0.0.1");
    ser_address->sin_port = htons(port);

    return 0;
}

int get_count(router_t *rt)
{
    size_t i;
    for(i = 0; i < 10; i++)
    {
        if( rt[i].source == 0 )
        {
            break;
        }
    }

    return i;
}

int update_router(router_t *rt1, router_t * rt2)
{
    int i, j, temp1 = 0 , off = 0, temp2 = 0;
    //int flag = 0;
    /*find the rest free array*/
    temp1 = get_count(rt1);
    //printf("temp1 %d\n", temp1);

    /*no need to continue, because all router info has been collected*/

    /*get the router info count*/
    temp2 = get_count(rt2);
    //printf("temp2 %d\n", temp2);

    for( j = 0; j < temp2; j++ )    //rt2
    {
        for( i = 0; i < temp1; i ++)  //rt1
        {
            if( rt1[i].source == rt2[j].source  && 
            rt1[i].neighbor == rt2[j].neighbor )
            {
                break;
            }
            else
            {
                continue;
            }

        }
  
        if( i == temp1 )
        {
            rt1[temp1 + off].source = rt2[j].source;
            rt1[temp1 + off].neighbor = rt2[j].neighbor;
            rt1[temp1 + off].distance = rt2[j].distance;

            off += 1;
        }
    }

    return 1;
}

/*here to input the distance between routers*/
void  init_router(router_t *rt, int port)
{
    //the flag to initialize the router table: statcic or input manually//
    int input_flag = 0;

    if ( port == 10000 )
    {
        puts("Initialize Router A\n");
        memset(rt, 0, 10 * sizeof(router_t));
        rt[0].source = 'a';
        rt[0].neighbor = 'b';
        
        rt[1].source = 'a';
        rt[1].neighbor = 'c';

        if( input_flag == 1 )
        {
            printf("Please input the distance between router a and b: "); 
            scanf("%d", &(rt[0].distance)); 
            //printf("%d\n", rt[0].distance);
            printf("Please input the distance between router a and c: "); 
            scanf("%d", &(rt[1].distance)); 
            //printf("%d\n", rt[1].distance);
        }
        else
        {
            rt[0].distance = 5;
            rt[1].distance = 5;
        }
    }

    else if( port == 10001 )
    {
        puts("Initialize Router B\n");
        memset(rt, 0, 10 * sizeof(router_t));
        rt[0].source = 'b';
        rt[0].neighbor = 'a';

        rt[1].source = 'b';
        rt[1].neighbor = 'c';

        if( input_flag == 1 )
        {
            printf("Please input the distance between router b and a: "); 
            scanf("%d", &(rt[0].distance)); 
            //printf("%d\n", rt[0].distance);
            printf("Please input the distance between router b and c: "); 
            scanf("%d", &(rt[1].distance)); 
            //printf("%d\n", rt[1].distance);
        }
        else
        {
            rt[0].distance = 5;
            rt[1].distance = 5;
        }
    }
    else if ( port == 10002 )
    {
        puts("Initialize Router C\n");
        memset(rt, 0, 10 * sizeof(router_t));
        rt[0].source = 'c';
        rt[0].neighbor = 'a';
        
        rt[1].source = 'c';
        rt[1].neighbor = 'b';
        
        rt[2].source = 'c';
        rt[2].neighbor = 'd';

        if( input_flag == 1 )
        {
            printf("Please input the distance between router c and a: "); 
            scanf("%d", &(rt[0].distance)); 
            //printf("%d\n", rt[0].distance);
            printf("Please input the distance between router c and b: "); 
            scanf("%d", &(rt[1].distance)); 
            //printf("%d\n", rt[1].distance);
            printf("Please input the distance between router c and d: "); 
            scanf("%d", &(rt[2].distance)); 
        }
        else
        {
            rt[0].distance = 5;
            rt[1].distance = 5;
            rt[2].distance = 3;
        }
        
    }
    else if ( port == 10003 )
    {
        puts("Initialize Router D\n");
        memset(rt, 0, 10 * sizeof(router_t));
        rt[0].source = 'd';
        rt[0].neighbor = 'c';

        if( input_flag == 1 )
        {
            printf("Please input the distance between router d and c: "); 
            scanf("%d", &(rt[0].distance)); 
            //printf("%d\n", rt[0].distance);
        }
        else
        {
            rt[0].distance = 3;
        }
    }

    else
    {
        puts("Init failed! Invalid port.");
    }
}

/*listening ports: 10000, 10001, 10002, 10003*/
void *server(void *port)
{
    size_t i;
	//init the router table
	router = (router_t *)malloc(10 * sizeof(router_t));
    memset(router, 0, 10 * sizeof(router_t));
    printf("port: %d\n", *(int *)port);
	init_router(router, *(int *)port);

    //puts("in server thread");
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
    //printf("Server listening on port %d\n", *(int *)port);
    
    int conn_times = 0;
    while( true )
    {
    	clilen = sizeof(cli_address);
    	connfd = accept(serverfd, (struct sockaddr *)&cli_address, &clilen);
    	char *data = (char *)malloc(0x200);
        memset(data, 0, 0x200);

        //puts("\nnow receviving data from client");
        read(connfd, data, 0x200);
        rt = (router_t *)data;
    	
        /*puts("\nbefore updating...");
        for(i = 0; i < get_count(router); i ++)
        {
            printf("%c %c %d\n", router[i].source, router[i].neighbor, router[i].distance);
        }
        puts("receviving data..");
        for(i = 0; i < get_count(rt); i ++)
        {
            printf("%c %c %d\n", rt[i].source, rt[i].neighbor, rt[i].distance);
        }
        puts("end receviving\n");*/
    	//data = NULL;
    	update_router(router, rt);
        puts("\nafter updating...");
        for(i = 0; i < get_count(router); i ++)
        {
            printf("%c %c %d\n", router[i].source, router[i].neighbor, router[i].distance);
        }
    	free(data);
    	close(connfd);

        conn_times += 1;

        if( conn_times > 6 )
        {
            generate_graph_array(router, C);
        }
        
        
    }
    
    return "sucess";
}

void *client(void *port)
{
    size_t count, i;
    //printf("in clinet thread\n");
    int sockfd1, sockfd2, sockfd3;
    socklen_t len;
    //router_t *rt = (router_t *)malloc(sizeof(router_t));
    //printf("%d %d %d\n", router[1].source, router[1].neighbor, router[1].flag);
    int conport[4] = {10000, 10001, 10002, 10003};
    int res;
    struct sockaddr_in ser_address1, ser_address2, ser_address3;

    while( true )
    {
        switch( *(int *)port )
        {
            case 10000:  
                count = get_count(router);
            	//the first connecting
                make_addr(&ser_address1, 10001);
                sockfd1 = socket(AF_INET, SOCK_STREAM, 0);
                len = sizeof(ser_address1);
                res = connect(sockfd1, (struct sockaddr *)&ser_address1, len);
                if( res != 0 )
                {
                    perror("connected to 10001 failed!");
                    exit(-1);
                }
                
                write(sockfd1, router, count * sizeof(router_t));
				close(sockfd1);

				//the second connecting
                make_addr(&ser_address2, 10002);
                sockfd2 = socket(AF_INET, SOCK_STREAM, 0);
                len = sizeof(ser_address2);
                res = connect(sockfd2, (struct sockaddr *)&ser_address2, len);
                if( res != 0 )
                {
                    perror("connected to 10002 failed!");
                    exit(-1);
                }

                /*sending router data*/
                write(sockfd2, router, count * sizeof(router_t));
                close(sockfd2);

                break;

            case 10001:
                count = get_count(router);
                make_addr(&ser_address1, 10000);
                sockfd1 = socket(AF_INET, SOCK_STREAM, 0);
                len = sizeof(ser_address1);
                res = connect(sockfd1, (struct sockaddr *)&ser_address1, len);
                if( res != 0 )
                {
                    perror("connected to 10000 failed!");
                    exit(-1);
                }

               
                write(sockfd1, router, count * sizeof(router_t));
                close(sockfd1);

                make_addr(&ser_address2, 10002);
                sockfd2 = socket(AF_INET, SOCK_STREAM, 0);
                len = sizeof(ser_address2);
                res = connect(sockfd2, (struct sockaddr *)&ser_address2, len);
                if( res != 0 )
                {
                    perror("connected to 10002 failed!");
                    exit(-1);
                }

                write(sockfd2, router, count * sizeof(router_t));
                close(sockfd2);

                break;

            case 10002:
                count = get_count(router);
                make_addr(&ser_address1, 10000);
                sockfd1 = socket(AF_INET, SOCK_STREAM, 0);
                len = sizeof(ser_address1);
                res = connect(sockfd1, (struct sockaddr *)&ser_address1, len);
                if( res != 0 )
                {
                    perror("connected to 10000 failed!");
                    exit(-1);
                }

                write(sockfd1, router, count * sizeof(router_t));
                close(sockfd1);

                make_addr(&ser_address2, 10001);
                sockfd2 = socket(AF_INET, SOCK_STREAM, 0);
                len = sizeof(ser_address2);
                res = connect(sockfd2, (struct sockaddr *)&ser_address2, len);
                if( res != 0 )
                {
                    perror("connected to 10001 failed!");
                    exit(-1);
                }

                write(sockfd2, router, count * sizeof(router_t));
                close(sockfd2);

                make_addr(&ser_address3, 10003);
                sockfd3 = socket(AF_INET, SOCK_STREAM, 0);
                len = sizeof(ser_address3);
                res = connect(sockfd3, (struct sockaddr *)&ser_address3, len);
                if( res != 0 )
                {
                    perror("connected to 10003 failed!");
                    exit(-1);
                }

                write(sockfd3, router, count * sizeof(router_t));

                close(sockfd3);

                break;

            case 10003:
                count = get_count(router);
                make_addr(&ser_address1, 10002);
                sockfd1 = socket(AF_INET, SOCK_STREAM, 0);
                len = sizeof(ser_address1);
                res = connect(sockfd1, (struct sockaddr *)&ser_address1, len);
                if( res != 0 )
                {
                    perror("connected to 10002 failed!");
                    exit(-1);
                }

                write(sockfd1, router, count * sizeof(router_t));

                close(sockfd1);
                //write(sockfd1, rt, sizeof(router_t));
                break;

            default: puts("invalid port!\n");
        }
        

        sleep(2);
    }

    return "success";
}
