#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#define menu "menu.txt"
#define BUFF_SIZE 400

// defining a struct which stores all_orders from a table, data? , flag? 
typedef struct {
    int flag; 
    int data;
    int all_orders[50];
} SharedData;

// reads the memu text file
void read_menu()
{
    FILE *menu_file = fopen(menu, "r");
    if (menu_file == NULL)
    {
        perror("Error opening menu file");
        exit(EXIT_FAILURE);
    }
    char buffer[1000];
    while (fgets(buffer, sizeof(buffer), menu_file) != NULL)
    {
        int len = strlen(buffer);
        if (buffer[len - 1] != '\0')
            buffer[len - 1] = '\0';
        printf("%s\n", buffer);
    }
    fclose(menu_file);
}

int take_customer_order(int customer_order[]){
    int ind = 0;
    int customer_bill = 0;
    while (1)
    {
        int inp = 0;
        scanf("%d", &inp);
        if(inp == -1)break;
        FILE *file = fopen(menu, "r");
        char buffer[1000];
        int cur = 0;
        while (fgets(buffer, sizeof(buffer), file) != NULL) cur++;
        if(inp>cur|| inp==0)return -1;
        customer_order[ind++] = inp;
    }
    return 0;
}

int main()
{
    int tableNo, noofCustomers, billAmount;
    printf("Enter Table Number: ");
    scanf("%d", &tableNo);


    
    while (1)
    {
        printf("Enter Number Of Customers: ");
        scanf("%d", &noofCustomers);
        //whileeee
        // menu will keep displaying to all the Customers
        if(noofCustomers != -1) 
        read_menu();
        
        // creating key
        key_t tableid = ftok("waiter.c", tableNo);
        if (tableid == -1)
        {
            printf("Error in executing ftok\n");
            exit(-1);
        }



        // creating key
        key_t table_manager = ftok("table.c", tableNo);
        if (table_manager == -1)
        {
            printf("Error in executing ftok\n");
            exit(-1);
        }

        // getting shmid of already created shm
        int table_mn_shmid = shmget(table_manager, 10, 0666 | IPC_CREAT);
        if (table_mn_shmid == -1)
        {
            printf("Error in executing shmget\n");
            exit(-2);
        }

        int *table_mn_shmptr = (int*)shmat(table_mn_shmid, NULL, 0);
        if(table_mn_shmptr==(void*)-1)
        {
            perror("Error in shmPtr in attaching the memory segment\n");
            exit(-1);
        }
        table_mn_shmptr[0] = 0;

        // intializing all orders to 0
        int all_orders[50];
        memset(all_orders,0,sizeof(all_orders));
        
        // creating N pipes for communication between table and all N Customers
        for (int i = 0; i < noofCustomers; i++)
        {

            int pipefd[2];
            if (pipe(pipefd) == -1)
            {
                perror("pipe");
                exit(-1);
            }
            
            // forking to create customers
            pid_t pid = fork();
            if (pid == -1)
            {
                perror("Error in creating fork");
                exit(-1);
            }
            else if (pid == 0) // child process
            { 
                int valid_order = -1;
                int customer_order[10] = {};
                printf("Enter the serial number(s) of the item(s) to order from the menu. Enter -1 when done: ");
                valid_order = take_customer_order(customer_order);
                while(valid_order == -1){
                    printf("Please Enter a Valid Order.\n");
                    printf("Enter the serial number(s) of the item(s) to order from the menu. Enter -1 when done: ");
                    valid_order = take_customer_order(customer_order);
                }
                close(pipefd[0]); // Close read end of the pipe
                write(pipefd[1],customer_order,40);
                close(pipefd[1]); // Close write end of the pipe
                exit(-1);
            }
            else // parent process
            {         
                close(pipefd[1]); // Close write end of the pipe
                int customer_order[10] = {};
                read(pipefd[0],customer_order,40);
                close(pipefd[0]); // Close read end of the pipe
                for(int j=0;j<10;j++){
                    all_orders[i*10 + j] = customer_order[j];
                }
            }
        }
        
        // getting shmid of already created shm
        int shmid = shmget(tableid, sizeof(SharedData), 0666 | IPC_CREAT);
        if (shmid == -1)
        {
            printf("Error in executing shmget\n");
            exit(-2);
        }

        
        
        // attaching table to the shm
        SharedData *shmptr = (SharedData *)shmat(shmid, NULL, 0); 
        
        while(shmptr->flag !=0);
        
        for(int j=0;j<50;j++){
            shmptr->all_orders[j] = all_orders[j];
        }
        
        if(noofCustomers == -1) {
            shmptr->data = -1;
            shmptr->flag = 1;
        }
        
        else
        { 
            shmptr->data = 1;
            shmptr->flag = 1;
        
            while(shmptr->flag != 3);
            
            printf("The total bill amount is %d INR\n",shmptr->data);
            shmptr->flag = 0;
        }

        table_mn_shmptr[0]=1;
        
        if (shmdt(shmptr) == -1)
        {
            perror("Error in shmdt\n");
            exit(-1);
        }
        if (shmdt(table_mn_shmptr) == -1)
        {
            perror("Error in shmdt\n");
            exit(-1);
        }
        
        if(noofCustomers == -1)break;
    }

    return 0;
}