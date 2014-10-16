#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


//Splits the command string so that it is in an acceptable input for execvp().
//http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/fork/exec.html referenced in creating a way to split the command.
int split(char *command, char **cmd)
{
	int background = 0;
	while (*command != '\0') {
		while(*command == ' '){
			*command++ ='\0';
		}
		if(*command == '&'){
			background = 1;
			*command++ = '\0';
		}else{
			*cmd++ = command;
		}
		while(*command != '\0' && *command != ' ' && *command != '&'){
			*command++;
		}
	*cmd = '\0';
	}
	return background;
}

//Function to check for terminated processes that is run frequently.
void checkBg(){
	pid_t childPID;
	int status;
	int i=0;
	//Checks to see if any child processes are terminated.
	for(i; i<10; i++){
		childPID = waitpid(-1, &status, WNOHANG);
		if(childPID>0){
			if(WIFEXITED(status) != 0){
				printf("%d Child was exited\n",childPID);
			}else if(WIFSIGNALED(status)!=0){
				printf("%d Child was killed\n",childPID);
			}
		}
	}
}

int main(int argc, char **argv){
	
	//Determines which user is on the command line either default which is 308> or what ever the user enters with the -p argument.
	char user[20] = "308>";
	char *home = getenv("HOME");
	int returned;
	if(argc == 3){
		if(strcmp(argv[1], "-p") == 0){
			strcpy(user,argv[2]);
		}else{
			printf("Invalid argument given only argument is -p USERNAME\n");
		}
	}else if (argc == 1){
		//Nothing here.
	}else{
		printf("Invalid Number of arguments given only arguments are -p USERNAME\n");
	}
	//The continually running while loop that takes commands and executes them.
	while(1){
		char curDir[512];
		char *envVar;
		char *cmd[50];
		char command[20];
		int background = 0;
		checkBg();
		printf("\n%s",user);
		
		//Gets the commands entered.
		gets(command);
		
		//Takes the command and determines if it is exit or a background command.
		if(strcmp(command, "exit")==0){
			exit(0);
		}
		
		checkBg();
		
		background = split(command,cmd);
		
		checkBg();
		//Determines if it is a special command or a normal one.
		
		//Detemines if CD.
		if(strcmp(cmd[0], "cd")==0){
			if (cmd[1] == '\0'){
				returned = chdir(home);
			}else{
			returned = chdir(cmd[1]);
			}
			if(returned < 0){
				printf("Could not cd to directory\n");
			}else {
				printf("In directory %s\n",cmd[1]);
			}
		
		//Determines if PID
		}else if(strcmp(command, "pid")==0) {
			returned = getpid();
			if(returned < 0){
				printf("Could not get PID");
			}else{
				printf("PID : %d \n", getpid());
			}
		//Determines if PPID
		}else if(strcmp(command, "ppid")==0){
			returned = getppid();
			if(returned<0){
				printf("Could not get ppid");
			}else{
			printf("PPID : %d \n", getppid());
			}
		//Determines if pwd.
		}else if(strcmp(command, "pwd")==0) {
			returned = getcwd(curDir, sizeof(curDir));
			if(returned < 0){
				printf("Could not get current directory");
			} else {
				printf("Current Directory is : %s\n",curDir);
			}
		//Determines if set env variable or get env variable.
		}else if (strcmp(cmd[0], "set")==0 || strcmp(cmd[0], "get") == 0){
			if(strcmp(cmd[0],"get")==0){
				envVar = getenv(cmd[1]);
				printf("Enviroment variable is : %s\n",envVar);
			}else if(strcmp(cmd[0],"set")==0){
				setenv(envVar,cmd[2],1);
			}
			
		//If not a special command forks the process and executes it.
		}else{
			
			checkBg();
			pid_t pid = fork();
			
			if (pid > 0){
				
				checkBg();
				printf("PID of child : %d \n",pid);
				
				//Determines if background or regular child.
				if(background == 0){			
				checkBg();
					wait(pid);
					printf("%d Child Exited\n",pid);
				}else if(background == 1) {
					
					checkBg();
					continue;
				}
				
			//If it is a child process executes the command.
			} else if (pid == 0){		
				checkBg();
				execvp(cmd[0],cmd);
				printf("Error command not found \n");
				exit(1);
			}
			checkBg();		
		}
		checkBg();
	}
	
	return 0;

}
