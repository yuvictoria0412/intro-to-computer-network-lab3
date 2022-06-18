#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LENGTH 16
#define PORT 8080

char buf[1024];

typedef struct Data_sequence{
    int seq_num;
    char Data[LENGTH];
}Data_sequence;

typedef struct ack_pkt{
    int seq_num;
}ack_pkt;


int main(int argc , char *argv[])
{
    //===========================================
	// Todo: Create TCP socket and TCP connection
	//===========================================
    int s;
    struct sockaddr_in server;
    struct hostent *hostnm;
    unsigned short port;
    char send_buf[50];
    char rev_buf[50];

    if (argc != 3) {
        printf("too few arguments!\n");
        exit(1);
    }

    hostnm = gethostbyname(argv[1]);
    if (hostnm == (struct hostent *) 0) {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }

    port = (unsigned short) atoi(argv[2]);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket init failed\n");
        exit(3);
    }

    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
        tcperror("Connect()");
        exit(4);
    }

    //========================
	// Receive data / send ack
	//========================
    while(1){
        //=================================================================
        // Todo: 1. receive data and send ACKs with correct sequence number
        //       2. simulate packet loss (or you can implement in server.c)
        // Required format: 
        //       received: seq_num = [sequence number]
        //       loss: seq_num = [seq_num]
        //=================================================================

        // if (recv(s, buf, sizeof(buf), 0) < 0) {
        //     tcperror("Recv()");
        //     exit(5);
        // }
    }
    close(s);
    return 0;
}