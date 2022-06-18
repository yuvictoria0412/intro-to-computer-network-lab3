#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <netdb.h>
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
    int seq_num, cwnd;

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
        printf("socket connect failed\n");
        exit(4);
    }

    //========================
	// Receive data / send ack
	//========================
    while(1) {
        //=================================================================
        // Todo: 1. receive data and send ACKs with correct sequence number
        //       2. simulate packet loss (or you can implement in server.c)
        // Required format: 
        //       received: seq_num = [sequence number]
        //       loss: seq_num = [seq_num]
        //=================================================================
        char temp[50];
        bool flag = 0;

        if (recv(s, rev_buf, sizeof(rev_buf), 0) < 0) {
            printf("received failed\n");
            exit(5);
        }

        for (int i = 0, j = 0; rev_buf[i] != '\0'; i++) {
            if (rev_buf[i] == ',') {
                flag = 1;
                rev_buf[i] = '\0';
            }
            if (flag) {
                temp[j++] = rev_buf[i];
                rev_buf[i] = '\0';
            }
        }
        seq_num = atoi(rev_buf);
        cwnd = atoi(rev_buf);
        printf("received: seq_num = [%d]\n", seq_num);
        printf("cwnd = %d\n", cwnd);
    }

    close(s);
    printf("client ended successfully :-D");
    return 0;
}