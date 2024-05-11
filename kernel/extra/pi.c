#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <math.h>
#define BITS 15
#define PYTHAGORAS ((1 << BITS) * (1 << BITS))
// if BITS=4, X: 10000(16) Y: 10000(16)
// PYTHAGORAS = 16 * 16 = 256
struct msgbuf {
    long msg_type;                /* message type, must be > 0 */
    char msg_text[2*BITS + 1];    /* message data */
};

pthread_t tid[3];
int inside_circle_count = 0;
int total_points_count = 0;

// Message queue ID
int GENERATE, JUDGE, ANNOUNCE;
void *generate();
void *judge();
void *announce();

void stop_pi(int sig) {
    printf("Exit Calculating Pi!\n");
    // Kill threads
    for (int i = 0; i < 3; i++)
        pthread_kill(tid[i], SIGTERM);

    msgctl(GENERATE, IPC_RMID, NULL);
    msgctl(JUDGE, IPC_RMID, NULL);
    msgctl(ANNOUNCE, IPC_RMID, NULL);
}

void calc_pi() {
    signal(SIGINT, stop_pi);
    // Create Message Queue and fetch ID
    GENERATE = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    JUDGE = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    ANNOUNCE = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);

    pthread_create(&tid[0], NULL, generate, NULL);
    pthread_create(&tid[1], NULL, judge, NULL);
    pthread_create(&tid[2], NULL, announce, NULL);
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_join(tid[2], NULL);

    return;
}

void *generate() {
    srand(time(NULL));     // set random seed
    while (1) {
        // define msgbuf for message queue
        struct msgbuf judge_msg;
        judge_msg.msg_type = 1;
        // Generate in forms of 01011010 which will be split into two number
        // X: 0101 = 5 Y: 1010 = 10
        for (int i = 0; i < 2*BITS; i++) {
            judge_msg.msg_text[i] = (rand() % 2) + '0';
        }
        // NULL TERMINATION
        judge_msg.msg_text[2*BITS] = '\0';
        // Send generated number to judge message queue
        msgsnd(JUDGE, &judge_msg, sizeof(judge_msg), 0);
    }
}

void *judge() {
    while (1) {
        struct msgbuf judge_msg, announce_msg;
        double pythagoras;
        int x, y;

        msgrcv(JUDGE, &judge_msg, sizeof(judge_msg), 1, 0);

        y = strtol(judge_msg.msg_text + BITS, NULL, 2);
        // Add Null termination for X
        judge_msg.msg_text[BITS] = 0;
        x = strtol(judge_msg.msg_text, NULL, 2);
        // Calculate pythagoras
        // X^2 = 5*5 = 25 Y^2 = 10 * 10 = 100
        // PYTHAGORAS = 16 * 16 = 256
        pythagoras = (x * x) + (y * y);
        announce_msg.msg_type = 1;

        if (pythagoras < PYTHAGORAS)
            announce_msg.msg_text[0] = 'Y';
        else
            announce_msg.msg_text[0] = 'N';
        announce_msg.msg_text[1] = '\0';
        msgsnd(ANNOUNCE, &announce_msg, sizeof(announce_msg), 0);
    }
}

void *announce() {
    while (1) {
        // Receive 'Y' or 'N' from judge
        struct msgbuf announce_msg;
        msgrcv(ANNOUNCE, &announce_msg, sizeof(announce_msg), 1, 0);
        if (announce_msg.msg_text[0] == 'Y') {
            inside_circle_count++;
            total_points_count++;
        } else if (announce_msg.msg_text[0] == 'N') {
            total_points_count++;
        }

        if (total_points_count % 100000 == 0) {
            double pi = (double)(inside_circle_count * 4) / total_points_count;
            printf("4x%d/%d, Pi: %lf\n", inside_circle_count, total_points_count, pi);
        }
    }
}