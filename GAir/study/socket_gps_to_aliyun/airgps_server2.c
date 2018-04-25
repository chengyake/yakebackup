#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero


#define SERVER_PORT    4567 
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 1024

int loop=1;
int new_server_socket=0;

int print_all() {

    int i, len=0;
	unsigned char buf[1600]={0};
    len = recv(new_server_socket , &buf[0], 1600, 0);
    if(len<=0) {
        loop=0;
        close(new_server_socket);
        printf("socket disconnected...\n");
    }
    printf("%s\n", buf);
    return 0;
}


void read_server() {
    int ret;
    static fd_set fd_sets;
    while(loop) {

        FD_ZERO(&fd_sets);
        FD_SET(new_server_socket , &fd_sets);


        ret = select(new_server_socket +1, &fd_sets, NULL, NULL, NULL); 
        if(ret < 0) {
            printf( "return select\n");
        }

        print_all();
    }
}

int start_read_server() {
    int ret;
	pthread_t pid;

	ret=pthread_create(&pid, NULL, (void *)read_server, NULL);
	if(ret != 0) {
        printf( "pthread create error: %d\n", ret);
    }

    return ret;
}



int main(int argc, char **argv)
{
    int i;
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    int server_socket = socket(PF_INET,SOCK_STREAM,0);
    if( server_socket < 0)
    {
        printf("Create Socket Failed!");
        exit(1);
    }
    { 
        int opt =1;
        setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    }

    if( bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
        printf("Server Bind Port : %d Failed!", SERVER_PORT); 
        exit(1);
    }

    if ( listen(server_socket, LENGTH_OF_LISTEN_QUEUE) )
    {
        printf("Server Listen Failed!"); 
        exit(1);
    }
    while (1) 
    {
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
        
        new_server_socket = accept(server_socket,(struct sockaddr*)&client_addr,&length);
        //printf("socket connected...\n");
        if ( new_server_socket < 0)
        {   new_server_socket=0;
            printf("Server Accept Failed!\n");
            break;
        }


        start_read_server();
    }
    //¹Ø±Õ¼àÌýÓÃµÄsocket
    close(server_socket);
    return 0;
}


