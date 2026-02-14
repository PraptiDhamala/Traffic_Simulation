#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define MAX 50
#define ROADS 4
#define HASH_SIZE 1000
struct VehicleStat {
    int id;
    int total_wait;
    int active; 
};

struct VehicleStat hashTable[HASH_SIZE];

int get_hash(int id) { return id % HASH_SIZE; }

void update_hash_map(int id, int wait) {
    int idx = get_hash(id);
    while (hashTable[idx].active && hashTable[idx].id != id) {
        idx = (idx + 1) % HASH_SIZE;
    }
    hashTable[idx].id = id;
    hashTable[idx].total_wait = wait;
    hashTable[idx].active = 1;
}

void get_worst_car(int *worst_id, int *max_wait) {
    *max_wait = -1;
    *worst_id = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        if (hashTable[i].active && hashTable[i].total_wait > *max_wait) {
            *max_wait = hashTable[i].total_wait;
            *worst_id = hashTable[i].id;
        }
    }
}
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

int isEmpty(struct Queue *q) { return (q->front == -1); }
int isFull(struct Queue *q) { return (q->rear == MAX - 1); }
int getSize(struct Queue *q) { return (isEmpty(q)) ? 0 : q->rear - q->front + 1; }

void enqueue(struct Queue *q, struct vehicle v) {
    if (isFull(q)) return;

    if (isEmpty(q)) {
        q->front = q->rear = 0;
        q->vehicles[q->rear] = v;
        return;
    }

    if (v.priority == 1) { 
        int pos = q->front;
        while (pos <= q->rear && q->vehicles[pos].priority == 1) pos++;
        for (int i = q->rear; i >= pos; i--) q->vehicles[i + 1] = q->vehicles[i];
        q->vehicles[pos] = v;
        q->rear++;
    } else {
        q->rear++;
        q->vehicles[q->rear] = v;
    }
}

int selectBestRoad(struct Queue roads[]) {
    int best = 0;
    int maxWait = -1;
    for (int i = 0; i < ROADS; i++) {
        int size = getSize(&roads[i]);
        if (size > maxWait) {
            maxWait = size;
            best = i;
        }
    }
    return best;
}
void printJsonData(struct Queue roads[], int greenRoad, int hour) {
    int worst_id, max_wait;
    get_worst_car(&worst_id, &max_wait); // Calculate analytics before printing

    printf("{\"hour\":%d,\"green\":%d,\"passed\":%d,\"worst_id\":%d,\"max_wait\":%d,\"roads\":[", 
            hour, greenRoad + 1, totalPassed, worst_id, max_wait);
    for (int i = 0; i < ROADS; i++) {
        printf("{\"size\":%d,\"vehicles\":[", getSize(&roads[i]));
        for (int j = roads[i].front; j <= roads[i].rear && j != -1; j++) {
            printf("%d%s", roads[i].vehicles[j].priority, (j < roads[i].rear ? "," : ""));
        }
        printf("]}%s", (i < ROADS - 1 ? "," : ""));
    }
    printf("]}\n");
    fflush(stdout);
}

int main() {
    srand(time(0));
    struct Queue roads[ROADS];
    for (int i = 0; i < ROADS; i++) initializeQueue(&roads[i]);
    
    // Initialize Hash Table
    for(int i = 0; i < HASH_SIZE; i++) hashTable[i].active = 0;

    int ticks = 0;
    int hour = 9; 

    while (1) {
        ticks++;
        if (ticks % 50 == 0) hour = (hour + 1) % 24;

        int spawnChance = (hour >= 17 && hour <= 19) ? 80 : 25;

        for (int i = 0; i < ROADS; i++) {
            if ((rand() % 100) < spawnChance) {
                // Generate vehicle with ID
                struct vehicle v = {rand() % 1000 + 1, (rand() % 10 == 0), 0};
                enqueue(&roads[i], v);
            }
            // Update wait times in Queues AND Hash Map
            for (int j = roads[i].front; j <= roads[i].rear && j != -1; j++) {
                roads[i].vehicles[j].waitingTime++;
                update_hash_map(roads[i].vehicles[j].id, roads[i].vehicles[j].waitingTime);
            }
        }

        int greenRoad = selectBestRoad(roads);

        for (int i = 0; i < 3; i++) {
            if (!isEmpty(&roads[greenRoad])) {
                roads[greenRoad].front++; 
                if (roads[greenRoad].front > roads[greenRoad].rear) 
                    initializeQueue(&roads[greenRoad]);
                totalPassed++;
            }
        }

        printJsonData(roads, greenRoad, hour);
        usleep(300000); 
    }
    return 0;
}