#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <sched.h>


unsigned long long starttime;
unsigned long long endtime;
unsigned long long waketime;
unsigned long long nanocalltime;


struct timeval tv;
struct timezone tz;

struct timespec rqtp;
struct timespec rmtp;

long long tv2usecs(struct timeval *tv){
	return tv->tv_sec * 1000000ULL + tv->tv_usec;
}


int main(){

	FILE *fp;
	char outputfilename[]="alarmtestfile";


	int retval;

	rqtp.tv_sec=0;
	rqtp.tv_nsec=0;

	unsigned long mask=1;
	unsigned int len = sizeof(mask);

	gettimeofday(&tv,&tz);
	starttime=tv2usecs(&tv);

	
	if (sched_setaffinity(0, len, &mask) < 0) {
    	perror("sched_setaffinity");
	}

	int pid;

//	printf("the attacker begins to run...\n");


	if((pid=fork())==0){

		execvp("./alarmvictim",NULL);
		perror("execvp");
		return 1;
	}

	//execvp("./alarmvictim",NULL);

	gettimeofday(&tv,&tz);
	nanocalltime=tv2usecs(&tv);

	retval=nanosleep(&rqtp,&rmtp);
//	if(retval==-1)
//		perror("nanosleep");

	gettimeofday(&tv,&tz);
	waketime=tv2usecs(&tv);
	
//	printf("retval==%ld\nrqtp= %ld", retval,rqtp.tv_nsec);
//	printf("!!!!!!!!!!!!!!!!!!!!! rmtp %ld\n", rmtp.tv_nsec);

	gettimeofday(&tv,&tz);
	endtime=tv2usecs(&tv);

	printf("attacker starts at %lld\nnanocall happens at %lld\nattacker wakes at %lld\nattacker ends at %lld\n",starttime,nanocalltime,waketime,endtime);
    
//	printf("nanocall happens at %lld\n",nanocalltime);

/*	fp=fopen(outputfilename,"a");
	if(fp==NULL){
		printf("Can't open output file %s!\n",outputfilename);
		exit(1);
	}

	fprintf(fp,"nanocall happens at %lld\n",nanocalltime);
	fclose(fp);
*/
	return 1;
}
