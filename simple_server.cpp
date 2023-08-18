#include "http_server.hh"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <queue>
#include <signal.h>


#define TOTAL_WORKER_THREADS 10
#define MAXM_SIZE_OF_QUEUE 500
using namespace std;

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_variable=PTHREAD_COND_INITIALIZER;
pthread_cond_t Q_full=PTHREAD_COND_INITIALIZER;

queue<int> request_queue;//socket fds queue
pthread_t threadIds[TOTAL_WORKER_THREADS];

int program_die=0;

void SIGINThandler(int x){
    program_die=1;
    pthread_cond_broadcast(&condition_variable);
    for(int i=0;i<TOTAL_WORKER_THREADS;i++)
    {
        pthread_join(threadIds[i],NULL);
    }
    
    exit(0);
}

void * workerthread(void *arg)
{
    int n;
    char buffer[1024];
    int newsockfd;
    
    while(program_die!=1)
    {   //acquire lock
        pthread_mutex_lock(&mutex);
            while(request_queue.empty())
            {
                pthread_cond_wait(&condition_variable,&mutex);
                if(program_die!=0){
                    pthread_mutex_unlock(&mutex);
                    pthread_exit(NULL);
                }

            }
            newsockfd = request_queue.front(); 
            request_queue.pop();
            if(request_queue.size() == MAXM_SIZE_OF_QUEUE - 1)
                {
                    pthread_cond_signal(&Q_full);
                }
        pthread_mutex_unlock(&mutex);
        //release lock

        
        //non-persistent connection (used while for part 1)
        //while(1)
        {
            bzero(buffer,sizeof(buffer));
            n = read(newsockfd,buffer,1023);
            if (n < 0)
            {
                close(newsockfd);
                printf("Error reading from socket.\n");
                //exit(1);
                continue;
            }
            else if( n == 0 )
            {
                close(newsockfd);
                //break;
                continue;
            }
    
            HTTP_Response*resp = handle_request(buffer);

            n=write(newsockfd,(resp->get_string()).c_str(),(resp->get_string()).length());
            if (n<=0)
            {
                close(newsockfd);
                printf("Error writing to the socket.\n");
                //exit(1);
                continue;
            }
            delete resp;
        }
        close(newsockfd);
        
    }
    
    pthread_exit(NULL);
	return NULL;
}

int main(int argc,char *argv[])
{
    int sockfd,newsockfd,portno;
    struct sockaddr_in server_address,client_address;
    socklen_t client_address_length;
    char buffer[1024];
    int n;
    

    signal(SIGINT,SIGINThandler);
    //creating worker pool
    for (int tmp=0 ; tmp < TOTAL_WORKER_THREADS ; tmp++)
    {
        pthread_create((threadIds+tmp),NULL,workerthread,NULL);
    }
    

    if(argc < 2){
        printf("Error : No port number provided.\n");
        exit(1);
    }

    sockfd=socket(AF_INET,SOCK_STREAM,0);
    //sockfd for listening

    if(sockfd<0){
        printf("Error opening socket.\n");
        exit(1);
    }

    bzero((char *)&server_address,sizeof(server_address));
    portno=atoi(argv[1]);//char to int

    server_address.sin_family=AF_INET;
    server_address.sin_addr.s_addr=INADDR_ANY;
    server_address.sin_port=htons(portno);

    if(bind(sockfd, (struct sockaddr*)&server_address,sizeof(server_address)) < 0)
    {
        printf("Error on binding socket.\n");
        SIGINThandler(0);
        //exit(1);
    }
    printf("Server Started\n");
    
    listen(sockfd,500);

    while(1)
    {
        client_address_length=sizeof(client_address);
        newsockfd=accept(sockfd,(struct sockaddr*)&client_address,&client_address_length);

        if ( newsockfd < 0 )
        {
            close(newsockfd);
            printf("Error on accept.\n");
            //exit(1);
            continue;
        }

        pthread_mutex_lock(&mutex);
            {
                
                if(request_queue.size() < MAXM_SIZE_OF_QUEUE)
                {
                    request_queue.push(newsockfd);
                    pthread_cond_signal(&condition_variable);
                }
                else
                {
                    close(newsockfd);
                    pthread_cond_wait(&Q_full,&mutex);
                }
            }
        pthread_mutex_unlock(&mutex);
    
    }
    return 0;

}
