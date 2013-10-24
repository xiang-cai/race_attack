#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>


struct stat buff;

unsigned long long starttime;
unsigned long long endtime;
double avg;

char *filenames[10000];
int count=100;
int rounds;

struct timeval tv;
struct timezone tz;

long long tv2usecs(struct timeval *tv){
	return tv->tv_sec * 1000000ULL + tv->tv_usec;
}

void load_filenames(char *namelistfile)
{
  FILE *f;
	
  if ((f = fopen(namelistfile, "r")) == NULL) {
    perror("Couldn't open filename list file");
    exit(1);
  }

  int nfilenames,i; 
  
  for (nfilenames=0;nfilenames<4000;nfilenames++;){
    filenames[nfilenames]=(char *)malloc(260*sizeof(char));
    fgets(filenames[nfilenames],260,f);
  }
  
  for (i = 0; i < 4000; i++){
  /*  if (close(open(filenames[i], O_CREAT, S_IRWXU))) {
	  perror("Couldn't create link");
	  exit(1);
	}
 */
  unlink(filenames[i]);	
  }
}

int main(int argc, char **argv){

	avg=0.0;
    int n=atoi(argv[2]);
    
	load_filenames(argv[1]);
	
	for (rounds=1;rounds<=count;rounds++){

		gettimeofday(&tv,&tz);
		starttime=tv2usecs(&tv);

		lstat(filenames[n],&buff);

		gettimeofday(&tv,&tz);
		endtime=tv2usecs(&tv);

		avg+=endtime-starttime;
	}


	avg= avg/count;
	printf("avg lstat time is %lf\n",avg);

	return 1;
}
