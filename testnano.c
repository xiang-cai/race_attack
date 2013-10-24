#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>

struct timespec rqtp;
struct timespec rmtp;

unsigned long long starttime;
unsigned long long endtime;

useconds_t utime;

int count=1;
int n;
double avg=0;

struct timeval tv;
struct timezone tz;

long long tv2usecs(struct timeval *tv){
	return tv->tv_sec * 1000000ULL + tv->tv_usec;
}


int main(int argc, char **argv){

	rqtp.tv_sec=0;
	rqtp.tv_nsec=atoi(argv[1]);
//	utime=atoi(argv[1]);
	
for (n=0;n<count;n++) {
	nanosleep(&rqtp,&rmtp);
	gettimeofday(&tv,&tz);
	starttime=tv2usecs(&tv);

	nanosleep(&rqtp,&rmtp);
//	usleep(utime);

	

	gettimeofday(&tv,&tz);
	endtime=tv2usecs(&tv);
	
//	printf("totally %lld\n",endtime-starttime);
	
	avg += endtime-starttime;
	}
//printf("totally %lld\n",endtime-starttime);
	double delay = avg/count;
//	double delay = avg/count-utime;
	printf("avg delay time is %lf\n",delay);
	return 1;
}
