#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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

int *queue;
int size = 100, front = -1, rear = -1;

void enqueue(int val) {
    if ((front == 0 && rear == size-1) || (rear == (front-1) % (size-1))) {
        printf("Queue is Full\n");
        exit(4);
    }
    else if (front == -1) {
        front = rear = 0;
        queue[rear] = val;
    }
    else if (rear == size-1 && front != 0) {
        rear = 0;
        queue[rear] = val;
    }
    else {
        rear++;
        queue[rear] = val;
    }
}

void dequeue() {
    if (front == -1) {
        printf("Queue is Empty\n");
        exit(5);
    }

    queue[front] = -1;

    if (front == rear) {
        front = -1;
        rear = -1;
    }
    else if (front == size-1)
        front = 0;
    else
        front++;
}

void displayQueue()
{
    if (front == -1)
    {
        printf("\nQueue is Empty");
        return;
    }
    printf("\nElements in Circular Queue are: ");
    if (rear >= front)
    {
        for (int i = front; i <= rear; i++)
            printf("%d ",queue[i]);
    }
    else
    {
        for (int i = front; i < size; i++)
            printf("%d ", queuequeue[i]);
 
        for (int i = 0; i <= rear; i++)
            printf("%d ", queue[i]);
    }
}

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
    int old_cwnd;
    
    bool flag = 0;
    bool loss = 0;
    time_t t;

    srand((unsigned) time(&t));
    queue = (int*)malloc(100 * sizeof(int));
    if (argc != 3) {
        printf("too few arguments!\n");
        exit(1);
    }

    hostnm = gethostbyname(argv[1]);
    if (hostnm == (struct hostent *) 0) {
        printf("Gethostbyname failed\n");
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
    int cnt = 50;
    while(cnt--) {
        //=================================================================
        // Todo: 1. receive data and send ACKs with correct sequence number
        //       2. simulate packet loss (or you can implement in server.c)
        // Required format: 
        //       received: seq_num = [sequence number]
        //       loss: seq_num = [seq_num]
        //=================================================================
        char temp[50];
        flag = 0;
        loss = 0;

        if (recv(s, rev_buf, sizeof(rev_buf), 0) < 0) {
            printf("received failed\n");
            exit(5);
        }

        // translate receive segment
        for (int i = 0, j = 0; i < 50; i++) {
            if (flag) {
                temp[j++] = rev_buf[i];
                rev_buf[i] = '\0';
            }
            if (rev_buf[i] == ',') {
                flag = 1;
                rev_buf[i] = '\0';
            }
        }
        temp[49] = '\0';
        seq_num = atoi(rev_buf);
        cwnd = atoi(temp);

        // cwnd
        if (old_cwnd != cwnd) {
            for (int i = 0; i < cwnd; i++) {
                enqueue(seq_num + i);
            }
        }

        // simulate packet loss
        if (rand() % 20 == 1)
            loss = 1;

        if (!loss) {
            // packet successfully received
            printf("received: seq_num = [%d]\n", seq_num);
            
            // send ACK back to server
            sprintf(send_buf, "%d", seq_num);
            if (send(s, send_buf, 50, 0) < 0) {
                printf("client send failed\n");
                exit(6);
            }
            // printf("client send ACK = %d\n", seq_num);
            printf("client send ACK = %d\n", queue[front]);
            if (queue[front] != seq_num) {
                printf("receive order error\n");
                displayQueue();
                exit(100);
            }
            // remove seq # from queue
            dequeue();
        }
        else {
            printf("loss: seq_num = [%d]", seq_num);
            dequeue();
            enqueue(seq_num);
        }
    }

    close(s);
    printf("client ended successfully :-D");
    return 0;
}