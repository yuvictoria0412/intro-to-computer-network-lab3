#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LENGTH 16
int ROUND = 20;

//======================================
// Data sequence
// sample sequence packet:
//    seq_num: sequence number
//    Data: data
// you can add another variables if need
//======================================
struct Data_sequence{
    int seq_num;
    char Data[LENGTH];
};

//================================
// ACK packet with sequence number
//================================
struct ack_pkt{
    int seq_num;
};

//======================
// Parameter declaration
//======================
enum STATE{SLOW_START, CONGESTION_AVOID};
unsigned short port;       /* port server binds to                */
//char buf[12];              /* buffer for sending & receiving data */
struct sockaddr_in client; /* client address information          */
struct sockaddr_in server; /* server address information          */
int s;                     /* socket for accepting connections    */
int ns;                    /* socket connected to client          */
int namelen;               /* length of client name               */
// int sockfd = 0;
// struct sockaddr_in serverInfo;
// int clientSockfd;
// struct sockaddr_in clientInfo;
// struct Data_sequence data_seq;
// struct ack_pkt ACK;
int cwnd = 1;
int ssthresh = 8;
int internet_state = SLOW_START;
int exp_seqnum;
char rev_buf[50];
char send_buf[50];

void print_state(int x) {
    switch (x) {
        case 0:
            printf("state: slow start\n");
            break;
        case 1:
            printf("state: congestion avoid\n");
            break;
    }
}


void sender() {
    //===========================================
    // Write your send here
    // Todo: 1. send cwnd sequences of data that
    //          starts with right sequence number
    // Required format: 
    //       send: seq_num = [sequence number]
    //===========================================
    static int seq_num = 1;

    exp_seqnum = seq_num + cwnd;
    while (seq_num < exp_seqnum) {  // send a sequence of segments
        sprintf(send_buf, "%d,%d", seq_num, cwnd);
        if (send(ns, send_buf, 50, 0) < 0) {
            printf("server send failed\n");
            exit(6);
        }
        printf("send: seg_num = [%d]\n", seq_num);
        seq_num++;
    }
}

void receiver() {
    //====================================================
    // Write your recv here
    // Todo: 1. receive ACKs from client
    //       2. detect if 3-duplicate occurs
    //       3. update cwnd and ssthresh
    //       4. remember to print the above information
    // Required format: 
    //       ACK: [sequence number]
    //       3-duplicate ACKs: seq_num = [sequence number]
    //       cwnd = [cwnd], ssthresh = [ssthresh]
    //====================================================
    static int previous_ack;
    static int same_ack = 1;
    static int resent_ack = -1;
    int current_ack;
    int receive_time = cwnd;

    while (receive_time--) {
        if (recv(ns, rev_buf, 50, 0) < 0) {
            printf("received failed\n");
            exit(7);
        }
        current_ack = atoi(rev_buf);
        if (current_ack > 0)
            printf("ACK get: [%d]\n", current_ack);
        else printf("loss ACK get: [%d]\n", -1*current_ack);

        if (current_ack == previous_ack && current_ack != resent_ack) {
            same_ack++;
            if (same_ack == 3) {    // 3-duplicate ack happens
                // update ssthresh and cwnd
                ssthresh = 0.5 * cwnd;
                if (!ssthresh) ssthresh = 1;
                cwnd = 1;
                same_ack = 0;
                //
                // exp_seqnum = seq_num + cwnd;
                //
                printf("3-duplicate ACKs: seq_num = [%d]\n", previous_ack);
                printf("cwnd = [%d], ssthresh = [%d]\n", cwnd, ssthresh);

                // resend duplicate acks
                sprintf(send_buf, "%d", previous_ack);
                resent_ack = previous_ack;
                if (send(ns, send_buf, 50, 0) < 0) {
                    printf("server resend duplicated segment failed\n");
                    exit(6);
                }
                printf("send: seg_num = [%d]\n", previous_ack);
                receive_time++;
                // update state
                internet_state = SLOW_START;
            }
        }
        else {
            same_ack = 1;
            previous_ack = current_ack;
        }
    }
    
    // if (current_ack == exp_seqnum) {
        // update cwnd
        if (cwnd >= ssthresh) {
            internet_state = CONGESTION_AVOID;
        }

        switch(internet_state) {
            case SLOW_START:
                cwnd += cwnd;
                break;
            
            case CONGESTION_AVOID:
                cwnd++;
                break;
        }
    // }

    print_state(internet_state);
}

int main(int argc, char *argv[])
{
    //===========================================
	// Todo: Create TCP socket and TCP connection
	//===========================================
    

    if (argc != 2) {
        printf("argument error\n");
        exit(1);
    }

    port = (unsigned short) atoi(argv[1]);

    // get a soclet for accepting connections
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("init socket failed\n");
        exit(2);
    }

    server.sin_family = AF_INET;
    server.sin_port   = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("bind failed\n");
        exit(3);
    }

    if (listen(s, 1) != 0) {
        printf("listen failed\n");
        exit(4);
    }

    namelen = sizeof(client);
    if ((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
    {
        printf("accept failed\n");
        exit(5);
    }
 
    //============================================================================
	// Start data transmission
    // To be simple, we receive ACKs after all cwnd sequences of data are sent.
    // Thus we can not react to 3-duplicate immediately, which is OK for this lab.
	//============================================================================
    print_state(internet_state);
    
    while (ROUND--) {
        sender();
        receiver();
    }

    close(ns);
    close(s);
    printf("server ended successfully :-D");
    return 0;
}