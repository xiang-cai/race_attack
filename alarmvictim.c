#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>


struct stat buff;
int fd;
int rounds=1;
unsigned long long starttime;
unsigned long long openstart;
unsigned long long openend;
unsigned long long endtime;


struct timeval tv;
struct timezone tz;

long long tv2usecs(struct timeval *tv){
	return tv->tv_sec * 1000000ULL + tv->tv_usec;
}


int main(){

	FILE* fp;
	char outputfilename[]="alarmtestfile";

	gettimeofday(&tv,&tz);
	starttime=tv2usecs(&tv);


//	printf("the victim begins to run...\n");
	
	gettimeofday(&tv,&tz);
	openstart=tv2usecs(&tv);

	for(rounds=1;rounds<=5000;rounds++){
		lstat("./hashlog",&buff);
	}

	gettimeofday(&tv,&tz);
	openend=tv2usecs(&tv);

//	printf("the victim ends\n");

	gettimeofday(&tv,&tz);
	endtime=tv2usecs(&tv);


	printf("victim starts at %lld\nlstat starts at %lld\nlstat finishes at %lld\nvictim ends at %lld\n",starttime,openstart,openend,endtime);

	printf("victim takes totally %lld\n",endtime-starttime);

//	printf("rounds executed is %d\n",rounds);

//	printf("victim starts at%lld\n",starttime);

/*
	fp=fopen(outputfilename,"a");
	if(fp==NULL){
		printf("Can't open output file %s!\n",outputfilename);
		exit(1);
	}

	fprintf(fp,"victim starts at%lld\n",starttime);
	fclose(fp);
*/
	return 1;
}
