#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>



int status;
pid_t chpid,w;


int main(int argc, char **argv){
	
	printf("parent begins\n");
	
	char* para[2];
	para[0]="./testforkch";
	para[1]="1";
	
	printf("para[0] is %s\n",para[0]);
	
	int chpid=fork();
	
	if(chpid==0){
		
		execvp("./testforkch",para);
    	perror("execvp");
    	return 1;
	}
	
	else if (chpid == -1) {
		perror("Couldn't fork");
		exit(1);
		
	}
	
	waitpid(chpid, &status, WUNTRACED | WCONTINUED);
	
	chpid=fork();
	
	if(chpid==0){
		
		execvp("./testforkch",para);
    	perror("execvp");
    	return 1;
	}
	
	else if (chpid == -1) {
		perror("Couldn't fork");
		exit(1);
		
	}
	
	waitpid(chpid, &status, WUNTRACED | WCONTINUED);
	
	printf("parent ends\n");
	
}
