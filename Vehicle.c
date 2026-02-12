#include <stdio.h>
#include <stdlib.h>
#define MAX 50
int totalEntered = 0;
int totalPassed = 0;

struct vehicle{
    int id;
    int priority;
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
    printf("%d",q->vehicles[i].id);
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
int main()
{
    struct Queue roads[5];
    int currentRoad = 0;
for(int i = 0; i < 5; i++) {
    roads[i].front = -1;
    roads[i].rear = -1;
}

    for(int i=0;i<5;i++)
    {
        initializequeue(&roads[i]);
    }
int choices;
do
{
    printf("\n******************** TRAFFIC CONTROL SYSTEM ********************\n");
    printf("1. Add vehicle\n");
    printf("2. Pass vehicle\n");
    printf("3. Display Road Status \n");
    printf("4. Display Counter \n");
    printf("5. Exit now!!!\n");
    printf("Enter Your Choice\n");
if (scanf("%d", &choices) != 1) {
            printf("Invalid input! Please enter a number.\n");
            while(getchar() != '\n'); // Clear the buffer
            choices = 0;        // Set to a non-exit value to continue loop
            continue;
        }
    if(choices==1)
    { 
        int roadnumber;
        struct vehicle v = {0, 0};
        printf("Enter Road Number (1-5) : ");
        scanf("%d",&roadnumber);
        if(roadnumber<1 || roadnumber>5 )
        {
         printf("Invalid Road Number\n") ;  
         continue;
        }
        else{
            printf("Enter Vehicle ID\n");
            scanf("%d",&v.id);

            printf("\nEnter the priority i;e 0 if normal and 1 if emergency \n ");
            scanf("%d",&v.priority);
            enqueue(&roads[roadnumber-1],v);
        }
    }
    else if(choices==2)
    {
    int roadnumber;
        struct vehicle v;
        printf("Clearing Road %d: ",currentRoad+1); 
        dequeue(&roads[currentRoad+1]);
        currentRoad=(currentRoad+1)%5;

    }
    else if(choices==3)
{
    printf("\n******************** Road Status ******************** \n");
    for(int i=0;i<5;i++)
    {
        printf("Road %d ",i+1);
        displayqueue(&roads[i]);
        printf(" | ");
        showDensity(&roads[i]);
          printf("\n");

    }
}
 else if(choices==4)
{
 printf("Total Vehicles Entered: %d\n", totalEntered);
printf("Total Vehicles Passed: %d\n", totalPassed);
}
else if(choices==5)
{
    printf("\n******************** Exiting Traffic system ******************** \n");
    break;
}
else{
    printf("Invalid Choice \n");
}
}while(choices != 5);
return 0;
}