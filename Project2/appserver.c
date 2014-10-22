#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include "Bank.h"
#include "appserver.h"

void *workThread(void * args);
int split(char *command, char **cmd);
int accounts;
pthread_mutex_t fileLock;
char * file;
int ret;
int main(int argc, char *argv[]){
    if (argc != 4){
        printf("There should be four arguments.\n");
        exit(0);
    } 
        int workers = atoi(argv[1]);
        accounts = atoi(argv[2]);
        file = argv[3];
        int i;
        int transID = 0;
       pthread_mutex_init(&(fileLock), NULL);       
        //Setting up command list.
        CommandsList* cmdsList;
        cmdsList = (CommandsList*) malloc(sizeof(CommandsList));
        cmdsList->size = 0;
        cmdsList->head = NULL;
        cmdsList->tail = NULL;
       pthread_mutex_init(&(cmdsList->lock),NULL);
        //Initiallizaing accounts
        initialize_accounts(accounts);
        
        pthread_mutex_t acctLock[accounts];
        
        for (i = 0; i<accounts; i++){
            pthread_mutex_init(&(acctLock[i]), NULL);
        }
        pthread_t worker_tid[workers];
        //Initializing worker arguments.
        argList * threadArgs;
        threadArgs = (argList*) malloc(sizeof(argList));
        
        threadArgs->cmdsArg = cmdsList;
        threadArgs->mutArg = acctLock;
        threadArgs->outputArg = file;
        threadArgs->accountsArg = accounts;
        
        int thread_index[workers];
       //Initializing worker threads.
        for (i = 0; i<workers; i++){
             thread_index[i] = i;
             pthread_create(&worker_tid[i],NULL, workThread,(void*) threadArgs);
        }
        //Main loop
        while(1){
	    char command[355];
	    int background = 0;
            
            gets(command);
		
                //If there is no input continue.
                if (strcmp(command, "\0")==0){
                    continue;
                }
                
                //Increment the transaction ID.
                transID++;
                
                //Print the transaction ID to the console.
                printf("Request ID: %d\n",transID);
                
               

                //Set up the command node
                CommandNode * tranCmd = (CommandNode*) malloc(sizeof(CommandNode));
                //If the command is END create as many END commands as workers and put at end of command list. 
                 if (strcmp(command, "END")==0){
                 int i = 0;
                 while (i<workers){
                 tranCmd->next = NULL;
                 tranCmd->prev = NULL;
                 tranCmd->id = transID;
                 strcpy(tranCmd->cmd,command);
                
                pthread_mutex_lock(&(cmdsList->lock));
                if(cmdsList->size == 0){
                    cmdsList->head = tranCmd;
                    cmdsList->tail = tranCmd;
                    cmdsList->size = 1;
                } else if (cmdsList->size > 0){
                    tranCmd->prev=cmdsList->tail;
                    tranCmd->prev->next = tranCmd;
                    cmdsList->tail=tranCmd;
                    tranCmd->next=NULL;
                    cmdsList->size = cmdsList->size +1;
                }
                pthread_mutex_unlock(&(cmdsList->lock));
                i++; 
                }
                 
                 }
                 //If not end create command node and put at the end of command list.
                 tranCmd->next = NULL;
                 tranCmd->prev = NULL;
                 tranCmd->id = transID;
                 strcpy(tranCmd->cmd,command);
                
                pthread_mutex_lock(&(cmdsList->lock));
                if(cmdsList->size == 0){
                    cmdsList->head = tranCmd;
                    cmdsList->tail = tranCmd;
                    cmdsList->size = 1;
                } else if (cmdsList->size > 0){
                    tranCmd->prev=cmdsList->tail;
                    tranCmd->prev->next = tranCmd;
                    cmdsList->tail=tranCmd;
                    tranCmd->next=NULL;
                    cmdsList->size = cmdsList->size +1;
                }
                pthread_mutex_unlock(&(cmdsList->lock));
                
                //If command is end wait until threads exit then exit.
                if(strcmp(command, "END")==0){
                for (i = 0; i<workers; i++){
                     pthread_join(worker_tid[i],NULL);
                    }
                    free(cmdsList);
		    exit(0);
		}
        }
        return 0;
}
//The worker function for each worker thread.
void *workThread(void * args){
    argList * workerArgs = args;
    CommandsList * workList = workerArgs->cmdsArg;
    pthread_mutex_t * workLocks = workerArgs->mutArg;
    FILE * output;
    FILE * output2;
    int workAccounts = workerArgs->accountsArg;
    struct timeval finishTime;
    //The loop
    while (1){
    int avoidDuplicates[20];
    char *commandToRun[355];
    int bal;
    pthread_mutex_lock(&(workList->lock));
    //If there are no commands wait.
    if(workList->size == 0){
        pthread_mutex_unlock(&(workList->lock));
        usleep(500);
        continue;    
    }
    //Get command to run and remove it from command list.
    CommandNode * cmd = workList->head;
    
    workList->size--;
    
    if (workList->size == 0){
        workList->head = NULL;
        workList->tail = NULL;
    } else {
        workList->head = workList->head->next;
        workList->head->prev = NULL;
    }
    pthread_mutex_unlock(&(workList->lock));
    
    //Split the command into a string array.
    split(cmd->cmd, commandToRun);
    
    //Checking what the command is.
    if(strcmp(commandToRun[0],"CHECK")==0){
        //Ensure that the account is actually in existence.
        if(atoi(commandToRun[1])<=accounts){
            //Check that the account can be locked.
            while( !(pthread_mutex_lock(&(workLocks[atoi(commandToRun[1])]))==0)){
                usleep(500);
            }
            //Get balance.
            bal = read_account(atoi(commandToRun[1]));
            
            //Unlock the account.
            pthread_mutex_unlock(&(workLocks[atoi(commandToRun[1])]));
            
            //Get time of day.
            gettimeofday(&finishTime,NULL);
            
            //Check that the file can be locked.
            while( !(pthread_mutex_lock(&(fileLock))==0)){
                usleep(500);
            }
            
            //Write to the file.
            output = fopen(file,"a");
            fprintf(output,"%d BAL %d TIME %d.%06d %d.%06d\n",cmd->id,bal,(int)cmd->time.tv_sec,(int)cmd->time.tv_usec,(int)finishTime.tv_sec,(int)finishTime.tv_usec);
            fclose(output);
            
            //Unlock the file.
            pthread_mutex_unlock(&(fileLock));
           
        }else{
            //Prints if there is no account.
            printf("Request ID: %d No Such Account\n",cmd->id);
        }
    
    }else if(strcmp(commandToRun[0],"TRANS")==0){
        int i = 1;
        int j = 0;
        int duplicate;
        //Loop to run until there is no more transactions to do.
        while(commandToRun[i]!='\0'){
            //Make sure it is looking at the account.
            if(!((i%2)==0)){
                j = 0;
                duplicate = 0;
                
                //Checking if it is a duplicate of an account during the same transaction.
                while (j<(i/2)){
                    if(avoidDuplicates[j]==atoi(commandToRun[i])){
                       duplicate = 1;
                    }
                    j++;
                }
                
                //If it is a duplicate continue loop.
                if(duplicate == 1){
                    i++;
                    continue;
                }
                
                //Check if the account can be locked if it can add it to the duplicate list.
                    if(pthread_mutex_lock(&(workLocks[atoi(commandToRun[i])]))==0){
                        avoidDuplicates[(i/2)]=atoi(commandToRun[i]);
                    }else{
                        i = 0;
                        
                        //If the account can not be locked unlock all previous account locks of transaction.
                        while(!(avoidDuplicates[i] == '\0')){
                            pthread_mutex_unlock(&(workLocks[avoidDuplicates[i]]));
                            avoidDuplicates[i] == '\0';
                            i++;
                        }
                        i= 0;
                        //Wait before trying again.
                        usleep(500);
                    }
                }
            i++;
        }
        int done = 0;
        int amountTest =0;
        i = 1;
        
        //If there is still commands and no account has ISF.
        while(commandToRun[i]!='\0' && done == 0){
            if(!((i%2)==0)){
                
                //Checks if amount is able to be added or subtracted.
                amountTest = read_account(atoi(commandToRun[i])) + atoi(commandToRun[i+1]);
                if(amountTest<0){
                    done = 1;
                    continue;
                }
            }
            i++;
        }
        
        //If an account has ISF then it writes that to the file.
        if(done == 1){
            pthread_mutex_lock(&(fileLock));
            output = fopen(file,"a");
            gettimeofday(&finishTime,NULL);
            fprintf(output,"%d ISF %d TIME %d.%06d %d.%06d\n",cmd->id,atoi(commandToRun[i]),(int)cmd->time.tv_sec,(int)cmd->time.tv_usec,(int)finishTime.tv_sec,(int)finishTime.tv_usec);
            fclose(output);
            pthread_mutex_unlock(&(fileLock));
        }else{
            
            //If it is not ISF then the action asked for is completed.
            i = 1;
            while(commandToRun[i]!='\0' && done == 0){
            if(!((i%2)==0)){
                amountTest = read_account(atoi(commandToRun[i])) + atoi(commandToRun[i+1]);
                write_account(atoi(commandToRun[i]),amountTest);
                }
            i++;
        }
        
        //Locks file to write too and writes then unlocks
        pthread_mutex_lock(&(fileLock));
        output = fopen(file,"a");
        gettimeofday(&finishTime,NULL);
        fprintf(output,"%d OK TIME %d.%06d %d.%06d\n",cmd->id,(int)cmd->time.tv_sec,(int)cmd->time.tv_usec,(int)finishTime.tv_sec,(int)finishTime.tv_usec);
        fclose(output);
        pthread_mutex_unlock(&(fileLock));
    }
    i = 0;
    
    //Unlocks the accounts involved with the transactions
    while(!(avoidDuplicates[i] == '\0')){
                pthread_mutex_unlock(&(workLocks[avoidDuplicates[i]]));
            i++;
        }
        
    }else if(strcmp(commandToRun[0],"END")==0){
        //If the END command is done then exit the thread.
                    pthread_exit(&ret);
    }else{
        printf("Invalid Command Command must be of type CHECK, TRANS, or END\n");
    }
    
    
    
    }
    return NULL;
}

//Splits the command string so that it is in an acceptable input for execvp().
//http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/fork/exec.html referenced in creating a way to split the command.
int split(char *command, char **cmd)
{
	while (*command != '\0') {
		while(*command == ' '){
			*command++ ='\0';
		}
			*cmd++ = command;
		while(*command != '\0' && *command != ' '){
			*command++;
		}
	*cmd = '\0';
	}
}


