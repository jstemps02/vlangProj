#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h> 
#include <sys/wait.h>
#include <sys/shm.h>
#define BUFFSIZE 200

bool exitVar = false; //the exit variable for the loop
int statusReg; //the status for regular processes
int statusBack =0; //status for background processes

char backProc[30] = ""; //puts the most recent background status in here

//function for exceuting commands and printing the results
void funct(char com[], char arg[]){
	arg[strlen(arg) -1 ] = '\0';
	int i =  0;
	int l,k;
	bool amper = false;
	char printVar[100];
	int y = 0;
	int x = 0;
	char * fixedArg[10];
	for(l =0;l < 10; l++){
		fixedArg[l] = (char *) malloc (25);
	}
	
	char * token = strtok(arg, " ");
	for(x = 0; x < strlen(com); x++){
		fixedArg[0][x] = com[x];
	}
	y++;
	while(token != NULL){
		for(x = 0; x < strlen(token); x++){
			fixedArg[y][x] = token[x];
		}
		token = strtok(NULL, " ");
		y++;
	}
	if(strcmp(arg, "\n") == 0){
		y--;
	}
	fixedArg[y] = NULL;
	int a = 0;
	// for(a = 0; a < y; a++){
	// 	printf("Line:%d || %s\n",a, fixedArg[a]);
	// } Testing for line input
	if(strstr(fixedArg[y-1], "&") != NULL){
		amper = true;
		fixedArg[y-1] = NULL;
	}

	// printf("The command is %s\n", com);   //for testing purposes
	// printf("The arguments are %s\n", arg);    //for testing purposes
	if(strcmp(com, "exit") == 0){
		exitVar = true;
		exit;
	}
	else if (strcmp(com, "pid") == 0)	{
		printf("%d\n", getpid());
	}
	else if (strcmp(com, "ppid") == 0)	{
		printf("%d\n", getppid());
	}
	else if (strcmp(fixedArg[0], "cd") == 0 && (y < 2))	{
	//	printf("Going Home\n");
		if (chdir(getenv("HOME")) != 0) 
    		perror("chdir() to /usr failed");
	//	printf("%s\n", getcwd(printVar, 100));
	}
	else if (strcmp(fixedArg[0], "cd") == 0)	{
		int val = chdir(fixedArg[1]);
		if (val != 0) 
    		perror("chdir() to arg failed");
		// printf("%s\n", fixedArg[1]);
		// printf("%s\n", getcwd(printVar, 100));
	}
	else if (strcmp(com, "pwd") == 0)	{
		printf("%s\n", getcwd(printVar, 100));
	}
	else{
			//shared memory for sharing process name
			int shmid;		// Hold shared memory "file descriptor"
    		key_t key;		// Key to locate same memory in different processes
    		char *shm, *s;	
			int ret;
			int childStatus;
			char msg[30];
			key = IPC_PRIVATE;
			shmid = shmget(key, 30, IPC_CREAT | 0666);
			shm = shmat(shmid, NULL, 0);
			s = shm;
			//if there is a detected amersand, go here
		if(amper == true){
			if ((ret = fork()) < 0)
				perror("fork() error");
			if (ret == 0) {
				int i;
				for(i = 0; i < 30; i++){
					backProc[i] = fixedArg[0][i];
				}
				statusBack = execvp(fixedArg[0], fixedArg);// array of strings., null terminated
				if (statusBack == -1) {
					printf("Process did not terminate correctly\n");
					exit(1);
				} 	
				
				printf("This line will not be printed if execvp() runs correctly\n");

			}
			else{
				fprintf(stderr, "[%d] %s\n", ret, fixedArg[0]);
				waitpid(-1, &statusBack, WNOHANG);
				
			}
			
		}
		//if no ampersand, go here
		else{
			if ((ret = fork()) < 0)
				perror("fork() error");
			if (ret == 0) {
				
				fprintf(stderr, "[%d] %s\n", getpid(), fixedArg[0]);
				int j = 0;
				
				for (j = 0; j < 30; j++){
					s[i] = fixedArg[0][i];
					i++;
				}
				s[i] = 0;
				statusReg = execvp(fixedArg[0], fixedArg);// array of strings., null terminated
				if (statusReg == -1) {
					printf("Process did not terminate correctly\n");
					exit(1);
				} 	
				
				printf("This line will not be printed if execvp() runs correctly\n");

			}
			else{
				waitpid(-1, &statusReg, 0);
				    int i;
					for (i = 0; shm[i] != 0; i++){
						msg[i] = shm[i];
					}
					msg[i] = 0;
				printf("[%d] %s Exit %d\n", ret, msg, statusReg);

			}
		}
		
	}
	//free up all malloc'd space
	for(k =0;k < 10; k++){
		free(fixedArg[k]);
	}
	free(token);
}
// I was not able to get exit status of background processes to function

// void checkChild(char* backProc){
// 	if(waitpid(-1, &statusBack, WNOHANG) && WIFEXITED(statusBack)){
// 		printf("%s Exit %d", backProc, WSTOPSIG(statusBack));
// 	}
// 	
// }


int main(int argc, char **argv) {
int i;
int switchVar = 0;

char command[30];
char args[30];

for(i=0; i<argc; i++){
	switchVar = strcmp(argv[i], "-p");

	if(argc > 3){ //too many arguments
		return 0;
	}

	else if(i == 1 && switchVar == 0 && argc == 3){
		while(exitVar == false){//if the shell does have a given name
	//		checkChild(backProc); functionality does not fully function
			printf("%s", argv[2]);
			fscanf(stdin, "%s", command);
			fgets(args, 100, stdin);
			funct(command, args);
		}
	}
	else if(i == 1 && switchVar == 0){ //if the shell does NOT have a given name
		while(exitVar == false){
	//		checkChild(backProc); functionality does not fully function
			printf("308sh> "); //prints shell line
			fscanf(stdin, "%s", command); //get input
			fgets(args, 100, stdin);
			funct(command, args); //call function to run the program
		}
	}
	else if(i == 3){ //if p cannot be found
		printf("-p not found\n");
	}

}
}
