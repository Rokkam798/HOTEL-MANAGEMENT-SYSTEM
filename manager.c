#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#define Earnings "earnings.txt"
#define BUFF_SIZE 200
#define MAX_TABLES 10

void cleanup(char *shmptr) {
    if (shmdt(shmptr) == -1)
    {
        perror("Error in shmdt in line 13\n");
        exit(-1);
    }
}

int main(){
    char *hmadm_shmptr, hmadmbuff[BUFF_SIZE];
    int hmwbuff[2];   
    int hmadm_shmid;
    
    // creating key to connect to admin and hotel manager
    key_t key = ftok("admin.c", 0);
    if (key == -1) {
        printf("Error in executing ftok1\n");
        exit(-1);
    }
    
    // getting the shmid of the already created shm between admin and hotel manager
    hmadm_shmid = shmget(key, BUFF_SIZE, 0666 | IPC_CREAT);
    if (hmadm_shmid == -1) {
        printf("Error in executing shmget\n");
        exit(-2);
    }
    
    // attaching hotel manager to the shm between admin hotel manager
    hmadm_shmptr = (char*)shmat(hmadm_shmid, NULL, 0);
    if(hmadm_shmptr==(void*)-1)
    {
        perror("Error in shmPtr in attaching the memory segment\n");
        return 1;
    }

    int totalbill=0, waiter_wages=0, profit=0;
    int noofWaiters;
    printf("Enter Total Number Of Tables at the Hotel\n");
    scanf("%d",&noofWaiters);
    
    // storing the keys, shmid's and shmptr's between all waiters and Hotel manager
    key_t waiter[noofWaiters+1];
    int hmw_shmid[noofWaiters+1];
    int* hmw_shmptr[noofWaiters+1];

    key_t table[noofWaiters+1];
    int hmt_shmid[noofWaiters+1];
    int* hmt_shmptr[noofWaiters+1];
    
    
    if(1){
        // "w" signifies write which basically over writes all the previous text present in the text file
        FILE *fp = fopen(Earnings, "w");
        fclose(fp);
    }

    for(int i=1;i<=noofWaiters;i++)
    {
        // creating key to connect to waiter[i] and hotel manager
        waiter[i]=ftok("manager.c",i);
        if(waiter[i]==-1)
        {
            printf("Error in executing ftok2\n");
            exit(-1);
        }
        
        // creating an shm between waiter[i] and hotel manager
        hmw_shmid[i] = shmget(waiter[i], 10, 0666 | IPC_CREAT);
        
        if (hmw_shmid[i] == -1) {
            printf("Error in executing shmget\n");
            exit(-2);
        }
        
        // attaching hotel manager to the shm between waiter[i] and hotel manager
        // we are type casting it to int* because we are storing an array of 2 integers in the shm
        hmw_shmptr[i] = (int*)shmat(hmw_shmid[i], NULL, 0);
        if(hmw_shmptr[i]==(void*)-1)
        {
            perror("Error in shmPtr in attaching the memory segment\n");
            exit(-1);
        }




        table[i]=ftok("table.c",i);
        if(table[i]==-1)
        {
            printf("Error in executing ftok3\n");
            exit(-1);
        }
        
        // creating an shm between waiter[i] and hotel manager
        hmt_shmid[i] = shmget(table[i], 10, 0666 | IPC_CREAT);
        
        if (hmt_shmid[i] == -1) {
            printf("Error in executing shmget\n");
            exit(-2);
        }
        
        // attaching hotel manager to the shm between waiter[i] and hotel manager
        // we are type casting it to int* because we are storing an array of 2 integers in the shm
        hmt_shmptr[i] = (int*)shmat(hmt_shmid[i], NULL, 0);
        if(hmt_shmptr[i]==(void*)-1)
        {
            perror("Error in shmPtr in attaching the memory segment\n");
            exit(-1);
        }

        hmt_shmptr[i][0]=0;
        
        // hmw_shmptr[i][0] --> stores the tableno (or) waiterno (both are same)
        // hmw_shmptr[i][1] --> stores the bill of the ith table
        
        // intializing the tableno with -1 inorder to know when there is change in the value which means commucation is happening
        hmw_shmptr[i][0] = -1;
    }
    
    // all the while loop does is read from the shms of hotel manager
    int fg = 0;
    int ct=0;
    while(1) {
        
        // hmadm_shmptr[0] != '\0' implies admin wrote something in the shm
        if(hmadm_shmptr[0] != '\0') {
            wait((int*)50);
            // copying the message from shm to buffer and resetting the shm to null so that we can know if it sends anything else
            strcpy(hmadmbuff, hmadm_shmptr);
            printf("Admin has issued command: %s\n", hmadmbuff);
            memset(hmadm_shmptr, '\0', BUFF_SIZE);

            // if admin has sent the close msg, we then calc waiter_wages and net profit before finally closing the hotel
            if(strcmp(hmadmbuff, "close") == 0) {
                
                    while(1){
                        int res = 0;
                        for(int i=1;i<=noofWaiters;i++){
                            struct shmid_ds shminfo;
                            if (shmctl(hmt_shmid[i], IPC_STAT, &shminfo) == -1) {
                                perror("shmctl");
                                exit(EXIT_FAILURE);
                            }
                            // printf("%d\n",shminfo.shm_nattch);
                            res+=shminfo.shm_nattch;
                        }
                        if(res==noofWaiters)break;
                        // printf("%d\n",res);
                    }
                    


                    for(int i =1; i <= noofWaiters; i++) {
                        if(hmw_shmptr[i][0] != -1) { // hmw_shmptr[i][0] != -1 implies waiter[i] has communicated 
                        
                            int table_number = hmw_shmptr[i][0];
                            int table_bill = hmw_shmptr[i][1];
                            totalbill += table_bill;
                            
                            // Clear the shared memory for this waiter
                            memset(hmw_shmptr[i], -1, 2 * sizeof(int));
                            
                            // entering earnigs of the table X in earnings file
                            FILE *fp = fopen(Earnings, "a");
                            if(fp == NULL) {
                                printf("Error opening the earnings file\n");
                                return -1;
                            }
                            fprintf(fp, "Earnings from Table %d: %d\n", table_number, table_bill);
                            fclose(fp);
                        }
                    }
                
                
                    waiter_wages = 0.4 * totalbill;
                    profit = totalbill - waiter_wages;
                    
                    // cleanup
                    for(int i=1;i<=noofWaiters;i++)
                    {
                        // detaching hotel manager to the shm between hotel manager and all waiters
                        if (shmdt(hmw_shmptr[i]) == -1)
                        {
                            perror("Error in shmdt in line 120\n");
                            exit(-1);
                        }

                        
                        if (shmdt(hmt_shmptr[i]) == -1)
                        {
                            perror("Error in shmdt in line 120\n");
                            exit(-1);
                        }
                        
                        // freeing the shm after all the processes have been detached
                        if(shmctl(hmw_shmid[i],IPC_RMID,NULL)==-1)
                        {
                            perror("Error in executing shmctl in line 127\n");
                            exit(-1);
                        }

                        // freeing the shm after all the processes have been detached
                        if(shmctl(hmt_shmid[i],IPC_RMID,NULL)==-1)
                        {
                            perror("Error in executing shmctl in line 127\n");
                            exit(-1);
                        }
                    }
                    
                    // "a" signifies append which does not over write all the previous text but adds in the next line 
                    FILE *fp = fopen(Earnings, "a");
                    if(fp == NULL) {
                        printf("Error opening the earnings file\n");
                        return -1;
                    }
                    fprintf(fp, "Total Earnings: %d\n", totalbill);
                    fprintf(fp, "Waiter Wages: %d\n", waiter_wages);
                    fprintf(fp, "Net Profit: %d\n", profit);
                    fclose(fp); 
                    
                    // detaching hotel manager to the shm between admin hotel manager 
                    cleanup(hmadm_shmptr);
                    printf("Thank You For Visiting The Hotel!\n");
                    exit(0);

                }
                
        }
        else { // we check if any waiter has communicated with hotel manager
        
            for(int i =1; i <= noofWaiters; i++) {
                if(hmw_shmptr[i][0] != -1) { // hmw_shmptr[i][0] != -1 implies waiter[i] has communicated 
                
                    int table_number = hmw_shmptr[i][0];
                    int table_bill = hmw_shmptr[i][1];
                    totalbill += table_bill;
                    
                    // Clear the shared memory for this waiter
                    memset(hmw_shmptr[i], -1, 2 * sizeof(int));
                    
                    // entering earnigs of the table X in earnings file
                    FILE *fp = fopen(Earnings, "a");
                    if(fp == NULL) {
                        printf("Error opening the earnings file\n");
                        return -1;
                    }
                    fprintf(fp, "Earnings from Table %d: %d\n", table_number, table_bill);
                    fclose(fp);
                }
            }
        }
    }
}