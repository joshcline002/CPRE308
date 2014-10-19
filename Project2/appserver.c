#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include "Bank.h"
#include "appserver.h"

void *workThread(void * args);
int split(char *command, char **cmd);

pthread_mutex_t fileLock;

int main(int argc, char *argv[]){
    if (argc != 4){
        printf("There should be four arguments.\n");
        exit(0);
    } 
        int workers = atoi(argv[1]);
        int accounts = atoi(argv[2]);
        char * file = argv[3];
        int i;
        int transID = 0;
        printf("Here 1");
       pthread_mutex_init(&(fileLock), NULL);       
        
        CommandsList* cmdsList;
        cmdsList = (CommandsList*) malloc(sizeof(CommandsList));
        cmdsList->size = 0;
        cmdsList->head = NULL;
        cmdsList->foot = NULL;
        printf("Here 2");
       pthread_mutex_init(&(cmdsList->lock),NULL);
        
        initialize_accounts(accounts);
        
        pthread_mutex_t acctLock[accounts];
        
        for (i = 0; i<accounts; i++){
            pthread_mutex_init(&(acctLock[i]), NULL);
        }
        printf("Here 3");
        pthread_t worker_tid[workers];
        
        argList * threadArgs;
        threadArgs = (argList*) malloc(sizeof(argList));
        
        threadArgs->cmdsArg = cmdsList;
        threadArgs->mutArg = acctLock;
        threadArgs->outputArg = file;
        threadArgs->accountsArg = accounts;
        
        int thread_index[workers];
       
       for (i = 0; i<workers; i++){
            thread_index[i] = i;
            pthread_create(&worker_tid[i],NULL, workThread,(void*) threadArgs);
        }
        
        while(1){
            char *cmd[50];
	    char command[20];
	    int background = 0;
            
            gets(command);
		
		//Takes the command and determines if it is exit or a background command.
		if(strcmp(command, "exit")==0){
			exit(0);
		}
		
		printf("%s", command);
                
                if (strcmp(command, "\0")==0){
                    continue;
                }
                
                transID++;
                
                CommandNode * tranCmd = (CommandNode*) malloc(sizeof(CommandNode));
                
                tranCmd->next = NULL;
                tranCmd->prev = NULL;
                tranCmd->id = transID;
                strcpy(tranCmd->cmds,command);
        }
        
        
}

void *workThread(void * args){
    argList * workerArgs = args;
    CommandsList * workList = workerArgs->cmdsArg;
    pthread_mutex_t * workLocks = workerArgs->mutArg;
    char * workFile = workerArgs->outputArg;
    int workAccounts = *(workerArgs->accountsArg);
    
    while (1){
    pthread_mutex_lock(&(workList->lock));
    
    if(workList->size == 0){
        pthread_mutex_unlock(&(workList->lock));
        usleep(5000);
    }
    
    CommandNode * cmd = workList->head;
    
    workList->size--;
    
    if (workList->size == 0){
        workList->head = NULL;
        workList->foot = NULL;
    } else {
        workList->head = workList->head->next;
        workList->head->prev = NULL;
    }
    
    pthread_mutex_unlock(&(workList->lock));
    
    
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


