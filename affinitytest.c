#include <stdio.h>
/*
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
*/
#include <sched.h>

int main(){
	unsigned long mask=1;
	unsigned int len = sizeof(mask);
	printf("len is: %d\n", len);
	
	if (sched_setaffinity(0, len, &mask) < 0) {
    	perror("sched_setaffinity");
	}
	
	if (sched_getaffinity(0, len, &mask) < 0) {
    	perror("sched_getaffinity");
    	return -1;
    }
	printf("my affinity mask is: %08lx\n", mask);
}
