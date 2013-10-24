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

struct stat test;
struct timeval tv;

int nfiles;
int krounds;
pid_t victimpid;
char* filenames[1<<25];
char* namelistfile;



long long tv2usecs(struct timeval *tv)
{
	return tv->tv_sec * 1000000ULL + tv->tv_usec;
}


void load_filenames(char *namelistfile)
{
	FILE *f;
	
	if ((f = fopen(namelistfile, "r")) == NULL) {
		perror("Couldn't open filename list file");
		exit(1);
	}
	int nfilenames = 0;
	while(fscanf(f, "%as\n", &filenames[nfilenames]) != EOF)
		nfilenames++;
}


void switchto(pid_t victimpid, char *tgt)
{
/*

		if (victimpid && kill(victimpid, SIGSTOP)) {
			perror("Couldn't stop victim");
			exit(1);
		}
		gettimeofday(&tv, NULL);
		printf("%18lld STOPPED\n", tv2usecs(&tv));
		
*/	
		int n;
		unsigned long long x1;
		unsigned long long x2;
		

	
		gettimeofday(&tv, NULL);
		x1=tv2usecs(&tv);
		
		//lstat(filenames[nfiles-1],&test);
		
		gettimeofday(&tv, NULL);
		x2=tv2usecs(&tv);		
		//printf("filenames[n-1] before unlink costs %lld\n", x2-x1);

		for(n=nfiles*krounds;n<nfiles*(krounds+1)-1;n++)
			lstat(filenames[n],&test);
			 	
		unlink(filenames[nfiles-1]);
		
		
		gettimeofday(&tv, NULL);
		x1=tv2usecs(&tv);
		
		//lstat(filenames[nfiles-1],&test);
		
		gettimeofday(&tv, NULL);
		x2=tv2usecs(&tv);		
		//printf("filenames[n-1] after unlink costs %lld\n", x2-x1);
		
		
		
		
		if (link(tgt, filenames[nfiles-1])) {
			perror("Couldn't create link");
			exit(1);
		}
		
			
			
		gettimeofday(&tv, NULL);
		x1=tv2usecs(&tv);
		
		lstat(filenames[nfiles-1],&test);
		
		gettimeofday(&tv, NULL);
		x2=tv2usecs(&tv);		
		printf("filenames[n-1] after link costs %lld\n", x2-x1);


/*
		gettimeofday(&tv, NULL);
		printf("%18lld CONTED\n", tv2usecs(&tv));
		

		
		
		if (victimpid &&	kill(victimpid, SIGCONT)) {
			perror("Couldn't continue victim");
			exit(1);
		}
*/		

}



void lastswitchto(pid_t victimpid, char *tgt)
{

/*
		if (victimpid && kill(victimpid, SIGSTOP)) {
			perror("Couldn't stop victim");
			exit(1);
		}
		gettimeofday(&tv, NULL);
		printf("%18lld STOPPED\n", tv2usecs(&tv));
		

*/	
		int n;
		long long x1;
		long long x2;
		
			
		unlink(filenames[nfiles-1]);
		if (link(tgt, filenames[nfiles-1])) {
			perror("Couldn't create link");
			exit(1);
		}
		
//		krounds++;
		
		for(n=nfiles*krounds;n<nfiles*(krounds+1);n++)
			unlink(filenames[n]);
		
			
		
		gettimeofday(&tv, NULL);
		x1=tv2usecs(&tv);
		
		lstat(filenames[nfiles-1],&test);
		
		gettimeofday(&tv, NULL);
		x2=tv2usecs(&tv);		
		printf("filenames[n-1] after link in lastswitch costs %lld\n", x2-x1);
		
		


		gettimeofday(&tv, NULL);
//		printf("%18lld CONTED\n", tv2usecs(&tv));
		
/*
		
		
		if (victimpid &&	kill(victimpid, SIGCONT)) {
			perror("Couldn't continue victim");
			exit(1);
		}
 
*/ 
}

int main(int argc, char **argv){

	if(argc!=8){
		printf("switchto argc is %d error!!\n",argc);
	
		printf("argv[0] is %s\nargv[1] is %s\nargv[2] is %s\nargv[3] is %s\nargv[4] is %s\nargv[5] is %s\nargv[6] is %s\nargv[7] is %s\n",argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],argv[7]);

		exit(0);
	}

	nfiles=atoi(argv[1]);
	krounds=atoi(argv[2]);
	namelistfile=argv[3];
	victimpid=atoi(argv[6]);
	
	load_filenames(namelistfile);
	
	
	
	printf("argv[0] is %s\nargv[1] is %s\nargv[2] is %s\nargv[3] is %s\nargv[4] is %s\nargv[5] is %s\nargv[6] is %s\n",argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6]);
	
	if(strcmp(argv[5],"s")==0){
		printf("call switchto\n");
		switchto(victimpid, argv[4]);
	}
	else{
		printf("call lastswitchto\n");
		lastswitchto(victimpid, argv[4]);
	}	
	return 1;	

}
