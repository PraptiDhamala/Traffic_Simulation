#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define MAX 50
int totalEntered = 0;
int totalPassed = 0;

struct vehicle{
    int id;
    int priority;
    int waitingTime;

};
struct Queue{
    struct vehicle vehicles[MAX];
    int front;
    int rear;
};

void initializequeue(struct Queue *q)
{
    q->front=-1;
    q->rear=-1;

}
int isEmpty(struct Queue *q){
    if(q->front==-1){
        return 1;
    }
    return 0;
}
int isFull(struct Queue *q){
    if(q->rear==MAX-1){
        return 1;
    }
    return 0;
}

void enqueue(struct Queue *q, struct vehicle v){
    if(isFull(q)){
        printf("Cannot add more vehicle, road is full \n");
        return;
    }
        if(isEmpty(q)){
       q->front=0;
       q->rear=0;
       q->vehicles[q->rear]=v;
       return;
    }
     if(v.priority == 1){ 
        int pos = q->front;
        while(pos <= q->rear && q->vehicles[pos].priority == 1){
            pos++;
        }
        for(int i = q->rear; i >= pos; i--){
            q->vehicles[i+1] = q->vehicles[i];
        }
        q->vehicles[pos] = v;
        q->rear++;
        printf("Emergency vehicle %d added!\n", v.id);
    }
    else{

    q->rear++;
    q->vehicles[q->rear]=v;
    printf("Vehicle %d added successfully",v.id);
}
totalEntered++;
  }
void dequeue(struct Queue *q)
{
    if(isEmpty(q))
{
printf("No vehicles available in the road");
return;
}
struct vehicle v=q->vehicles[q->front];
printf("Vehicle %d passed the traffic \n",v.id);
    if(v.priority==1)
        printf("Emergency vehicle %d passed!\n", v.id);
    else
        printf("Vehicle %d passed!\n", v.id);

q->front++;
totalPassed++;
if(q->front>q->rear)
{
    q->front=-1;
    q->rear=-1;
}
}

void displayqueue(struct Queue *q)
{
    if (isEmpty(q))
{
printf("The road is Empty \n");
return;
}
for(int i=q->front; i<= q->rear; i++)
{
    printf("%d\n",q->vehicles[i].id);
}
}
void showDensity(struct Queue *q)
{
    if(isEmpty(q))
    {
        printf("Road is empty");
    }
     int size = q->rear - q->front + 1;
    if(size <= 3) printf("Low Traffic");
    else if(size <= 7) printf("Medium Traffic");
    else printf("High Traffic");
}
float avgWaitingTime(struct Queue *q){
    if(isEmpty(q)) return 0;
    int sum = 0;
    for(int i=q->front;i<=q->rear;i++)
        sum += q->vehicles[i].waitingTime;
    return (float)sum / (q->rear - q->front + 1);
}

void updateWaitingTime(struct Queue *q){
    if(isEmpty(q)) return;
    for(int i=q->front;i<=q->rear;i++)
        q->vehicles[i].waitingTime++;
}

struct vehicle generateRandomVehicle(){
    struct vehicle v;
    v.id = rand()%1000 + 1;
    v.priority = rand()%2;
    v.waitingTime = 0;
    return v;
}

int getSize(struct Queue *q) {
    if(isEmpty(q)) return 0;
    return q->rear - q->front + 1;
}

int getSmartSignal(struct Queue roads[]) {
    int max = -1;
    int index = 0;

    for(int i = 0; i < 5; i++) {
        int size = getSize(&roads[i]);
        if(size > max) {
            max = size;
            index = i;
        }
    }
    return index;
}

int checkEmergency(struct Queue roads[]) {
    for(int i = 0; i < 5; i++) {
        if(!isEmpty(&roads[i])) {
            if(roads[i].vehicles[roads[i].front].priority == 1) {
                return i;
            }
        }
    }
    return -1;
}
void exportToJSON(struct Queue roads[], int greenRoad) {

    FILE *fp = fopen("output.json", "w");

    fprintf(fp,
    "{\n"
    "\"road1\": %d,\n"
    "\"road2\": %d,\n"
    "\"road3\": %d,\n"
    "\"road4\": %d,\n"
    "\"road5\": %d,\n"
    "\"green\": %d\n"
    "}",
    getSize(&roads[0]),
    getSize(&roads[1]),
    getSize(&roads[2]),
    getSize(&roads[3]),
    getSize(&roads[4]),
    greenRoad + 1
    );

    fclose(fp);
}

int main() {

    srand(time(0));

    struct Queue roads[5];

    for(int i=0;i<5;i++)
        initializequeue(&roads[i]);

    int cycle = 0;

    while(1) {

        cycle++;
        printf("\n================ CYCLE %d ================\n", cycle);

        for(int i=0;i<5;i++) {

            int newVehicles;

            if(cycle % 20 < 10)
                newVehicles = rand()%4 + 2;  // heavy traffic
            else
                newVehicles = rand()%4;      // normal traffic

            for(int j=0;j<newVehicles;j++) {
                struct vehicle v = generateRandomVehicle();
                enqueue(&roads[i], v);
            }
        }

        for(int i=0;i<5;i++)
            updateWaitingTime(&roads[i]);

        int emergencyRoad = checkEmergency(roads);
        int currentRoad;

        if(emergencyRoad != -1) {
            currentRoad = emergencyRoad;
            printf("Emergency override! Road %d gets GREEN\n", currentRoad+1);
        }
        else {
            currentRoad = getSmartSignal(roads);
            printf("Smart Signal → Road %d gets GREEN\n", currentRoad+1);
        }

        // 4️Pass 3 Vehicles
        for(int i=0;i<3;i++)
            dequeue(&roads[currentRoad]);

        //Display Status
        for(int i=0;i<5;i++) {
            printf("Road %d: %d vehicles | Avg Wait: %.2f\n",
                i+1,
                getSize(&roads[i]),
                avgWaitingTime(&roads[i])
            );
        }

        printf("Total Entered: %d | Total Passed: %d\n",
                totalEntered, totalPassed);

        // Export JSON
        exportToJSON(roads, currentRoad);

        sleep(1);
    }

    return 0;
}

