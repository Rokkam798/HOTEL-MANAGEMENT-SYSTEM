#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <string.h>

#define menu "menu.txt"

// defining a struct which stores all_orders from a table, data? , flag? 
typedef struct {
    int flag; 
    int data;
    int all_orders[50];
} SharedData;

int take_customer_order(int item){
    
    if(item == 0)
    return 0;
    
    int customer_bill = 0;
    
    FILE *file = fopen(menu, "r");
    
    char buffer[1000];
    int cur = 0;
    int price = -1;
    
    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        // cur imples the line number in the menu file
        cur++;
        
        // finding the last second word which is the price in the line 
        if (cur == item)
        {
            char *words[256]; 
            char *word;
            int word_count = 0;
            word = strtok(buffer, " ");
            while (word != NULL)
            {
                words[word_count++] = word;
                word = strtok(NULL, " ");
            }
            if (word_count >= 2)
            {
                price =  atoi(words[word_count - 2]); 
            }
        }
    }
    
    customer_bill += price;
    
    fclose(file);
    
    return customer_bill;
}

void attachtoHM(int waiterno,int billAmount)
{
    // tableno = waiterno = X
    // creating key for waiter X to connect to hotel manager
    key_t waiterid=ftok("manager.c",waiterno);
    if (waiterid == -1) {
        printf("Error in executing ftok\n");
        exit(-1);
    }
    
    // getting the shmid of the already created shm between waiter X and hotel manager
    int hmw_shmid=shmget(waiterid, 10 , 0666 | IPC_CREAT);
    if (hmw_shmid == -1) {
        printf("Error in executing shmget\n");
        exit(-2);
    }
    
    // attaching waiter X to the shm between waiter X and hotel manager
    int* shmptr=(int*)shmat(hmw_shmid,NULL,0);
    if(shmptr==(void*)-1)
    {
        perror("Error in shmPtr in attaching the memory segment\n");
        exit(-1);
    }
    
    // intitializing the variables
    shmptr[0]=waiterno;
    shmptr[1]=billAmount;
    
    while(shmptr[0]!=waiterno);
}

int main(){
    int waiterno;
    printf("Enter Waiter ID: ");
    scanf("%d",&waiterno);

    // creating key
    key_t tableid = ftok("waiter.c", waiterno);
    if (tableid == -1) {
        printf("Error in executing ftok\n");
        exit(-1);
    }
    
    // getting the shmid of already created shm
    int shmid = shmget(tableid, sizeof(SharedData), 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        printf("Error in executing shmget\n");
        exit(-2);
    }
    
    while(1){
        
        // attaching waiter to the shm
        SharedData *shmptr = (SharedData *)shmat(shmid, NULL, 0); 
        
        while(shmptr->flag != 1);
        shmptr->flag = 2;
        int table_bill = 0;

        if(shmptr->data == -1)
        break;
        
        // as a table can have max 5 customers and each customer can order max 10 orderes, we can take max 50 orders per table
        if(shmptr->data == 1){
            for(int i=0;i<50;i++){
                // take_customer_order() takes the order and calculates bill of each customer
                table_bill += take_customer_order(shmptr->all_orders[i]);
            }
        }
        
        printf("Bill Amount for Table %d: %d INR\n",waiterno,table_bill);
        
        // waiter will attach to hotel manager and send waiterno and table_bill to hotel manager
        attachtoHM(waiterno,table_bill);
        
        shmptr->data = table_bill;
        shmptr->flag = 3;

        if (shmdt(shmptr) == -1)
        {
            perror("Error in shmdt\n");
            exit(-1);
        }
    }
    if(shmctl(shmid,IPC_RMID,NULL)==-1)
    {
        perror("Error in executing shmctl\n");
        exit(-1);
    }
 }