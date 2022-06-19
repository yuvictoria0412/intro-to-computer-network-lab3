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

typedef struct Q {
    int size;
    int front;
    int rear;
    int arr[100];
}queue;



void enqueue(int val, queue *q) {
    if ((q->front == 0 && q->rear == q->size-1) || (q->rear == (q->front-1) % (q->size-1))) {
        printf("Queue is Full\n");
        exit(4);
    }
    else if (q->front == -1) {
        q->front = q->rear = 0;
        q->arr[q->rear] = val;
    }
    else if (q->rear == q->size-1 && q->front != 0) {
        q->rear = 0;
        q->arr[q->rear] = val;
    }
    else {
        q->rear++;
        q->arr[q->rear] = val;
    }  
}

void dequeue(queue *q) {
    if (q->front == -1) {
        printf("Queue is Empty\n");
        exit(5);
    }

    q->arr[q->front] = -1;

    if (q->front == q->rear) {
        q->front = -1;
        q->rear = -1;
    }
    else if (q->front == q->size-1)
        q->front = 0;
    else
        q->front++;
}

void displayQueue(queue *q) {
    if (q->front == -1) {
        printf("Queue is Empty");
        return;
    }
    printf("Elements in Circular Queue are: ");
    if (q->rear >= q->front) {
        for (int i = q->front; i <= q->rear; i++)
            printf("%d ",q->arr[i]);
    }
    else {
        for (int i = q->front; i < q->size; i++)
            printf("%d ", q->arr[i]);

        for (int i = 0; i <= q->rear; i++)
            printf("%d ", q->arr[i]);
    }
    printf("\n");
}

bool queue_empty(queue *q) {
    return q->front == -1 && q->rear == -1;
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
    int old_cwnd = 0;
    queue a, b;
    queue* seq_queue = &a;
    queue* ack_queue = &b;
    a.size = b.size = 100;
    b.front = a.front = -1;
    b.rear = a.rear = -1;
    printf("%d", seq_queue->size);
    

    bool flag = 0;
    bool loss = 0;
    time_t t;

    srand((unsigned) time(&t));

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
            // printf("old cwnd = %d, cwnd = %d", old_cwnd, cwnd);
            old_cwnd = cwnd;

            for (int i = 0; i < cwnd; i++) {
                enqueue(seq_num + i, seq_queue);
            }
            
        }

        // simulate packet loss
        if (rand() % 30 == 1 && seq_num != ack_queue->arr[ack_queue->front])    // avoid same packet loss
            loss = 1;

        if (!loss) {
            // packet successfully received
            printf("received: seq_num = [%d]\n", seq_num);

            // send ACK back to server
            displayQueue(ack_queue);
            if (queue_empty(ack_queue)) {   // no loss happened before
                sprintf(send_buf, "%d", seq_num);
                printf("client send ACK = seq_num %d\n", seq_num);
            }
            else {  // loss happened before send duplicate ack
                if (seq_num == ack_queue->arr[ack_queue->front]) {
                    dequeue(ack_queue);
                    if (queue_empty(ack_queue)) {
                        printf("client send ACK = seq_num %d\n", seq_queue->arr[seq_queue->front]);
                        sprintf(send_buf, "%d", seq_queue->arr[seq_queue->front]);
                    }
                    else {
                        printf("client send ACK = seq_num %d\n", ack_queue->arr[ack_queue->front]);
                        sprintf(send_buf, "%d", ack_queue->arr[ack_queue->front]);
                    }
                }
                else {
                    sprintf(send_buf, "%d", ack_queue->arr[ack_queue->front]);
                    printf("client send ACK = duplicate ack %d\n", ack_queue->arr[ack_queue->front]);
                }
            }
            if (send(s, send_buf, 50, 0) < 0) {
                printf("client send failed\n");
                exit(6);
            }

            // receive
            if (seq_queue->arr[seq_queue->front] != seq_num) {
                printf("receive order error\n");
                displayQueue(seq_queue);
                exit(100);
            }
            // remove seq # from queue
            dequeue(seq_queue);
            displayQueue(seq_queue);
        }
        else {
            if (queue_empty(ack_queue)) {
                printf("loss: seq_num = [%d]\n", seq_num);
                sprintf(send_buf, "%d", -1);
                if (send(s, send_buf, 50, 0) < 0) {
                    printf("client send failed\n");
                    exit(6);
                }
                // exp_seq = seq_num;
            }
            else {
                sprintf(send_buf, "%d", ack_queue->arr[ack_queue->front]);
                // printf("client send ACK = duplicate ack %d\n", ack_queue->arr[ack_queue->front]);
                
            }
            enqueue(seq_num, ack_queue);
            dequeue(seq_queue);
            enqueue(seq_num, seq_queue);
        }
        printf("\n");
    }

    close(s);
    printf("client ended successfully :-D");
    return 0;
}