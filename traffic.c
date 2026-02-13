#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define MAX 50
#define ROADS 4

struct vehicle {
    int id;
    int priority;      
    int waitingTime;
};

struct Queue {
    struct vehicle vehicles[MAX];
    int front;
    int rear;
};

int totalPassed = 0;


void initializeQueue(struct Queue *q) {
    q->front = -1;
    q->rear = -1;
}

int isEmpty(struct Queue *q) {
    return (q->front == -1);
}

int isFull(struct Queue *q) {
    return (q->rear == MAX - 1);
}

int getSize(struct Queue *q) {
    if (isEmpty(q)) return 0;
    return q->rear - q->front + 1;
}

void enqueue(struct Queue *q, struct vehicle v) {
    if (isFull(q)) return;

    if (isEmpty(q)) {
        q->front = q->rear = 0;
        q->vehicles[q->rear] = v;
        return;
    }

    if (v.priority == 1) {
        int pos = q->front;
        while (pos <= q->rear && q->vehicles[pos].priority == 1)
            pos++;

        for (int i = q->rear; i >= pos; i--)
            q->vehicles[i + 1] = q->vehicles[i];

        q->vehicles[pos] = v;
        q->rear++;
    } else {
        q->rear++;
        q->vehicles[q->rear] = v;
    }
}

struct vehicle dequeue(struct Queue *q) {
    struct vehicle empty = {-1,0,0};

    if (isEmpty(q))
        return empty;

    struct vehicle v = q->vehicles[q->front];
    q->front++;

    if (q->front > q->rear)
        initializeQueue(q);

    totalPassed++;
    return v;
}

void updateWaitingTime(struct Queue *q) {
    if (isEmpty(q)) return;

    for (int i = q->front; i <= q->rear; i++)
        q->vehicles[i].waitingTime++;
}

float avgWaitingTime(struct Queue *q) {
    if (isEmpty(q)) return 0;

    int sum = 0;
    for (int i = q->front; i <= q->rear; i++)
        sum += q->vehicles[i].waitingTime;

    return (float)sum / getSize(q);
}


struct vehicle generateVehicle() {
    struct vehicle v;
    v.id = rand() % 1000 + 1;
    v.priority = rand() % 5 == 0 ? 1 : 0;  
    v.waitingTime = 0;
    return v;
}

int selectSignal(struct Queue roads[]) {
    int best = 0;
    int maxSize = 0;

    for (int i = 0; i < ROADS; i++) {
        int size = getSize(&roads[i]);
        if (size > maxSize) {
            maxSize = size;
            best = i;
        }
    }
    return best;
}

void printData(struct Queue roads[], int greenRoad) {
    printf("{");
    printf("\"roads\":[");
    for (int i = 0; i < ROADS; i++) {
        printf("{\"id\":%d,\"size\":%d,\"avg_wait\":%.2f,\"vehicles\":[", 
               i + 1, getSize(&roads[i]), avgWaitingTime(&roads[i]));
        
        for (int j = roads[i].front; j <= roads[i].rear && j != -1; j++) {
            printf("%d", roads[i].vehicles[j].priority);
            if (j < roads[i].rear) printf(",");
        }
        
        printf("]}");
        if (i < ROADS - 1) printf(",");
    }
    printf("],");
    printf("\"green\":%d,", greenRoad + 1);
    printf("\"passed\":%d", totalPassed);
    printf("}\n");
    fflush(stdout);
}

int main() {
    srand(time(0));
    struct Queue roads[ROADS];

    for (int i = 0; i < ROADS; i++)
        initializeQueue(&roads[i]);

    while (1) {
        for (int i = 0; i < ROADS; i++) {
            if ((rand() % 100) < 50) {   
                enqueue(&roads[i], generateVehicle());
            }
        }

        for (int i = 0; i < ROADS; i++)
            updateWaitingTime(&roads[i]);

        int greenRoad = selectSignal(roads);

        for (int i = 0; i < 5; i++) {
            if (!isEmpty(&roads[greenRoad])) {
                dequeue(&roads[greenRoad]);
            }
        }

        printData(roads, greenRoad);

        usleep(200000); 
    }
    return 0;
}