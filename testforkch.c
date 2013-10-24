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



int main(int argc, char **argv){
	
	int i;
	
//	int n=atoi(argv[1]);
	
	printf("child begins\n");
	
	printf("argv[1] is %s\n", argv[1]);
//	printf("n = %d\n",n);
	
	for(i=1;i<10000;i++);
	
//	if(1==n)
//		printf("i get 1\n");
	
	
	printf("child ends\n");
	
	return;
}
