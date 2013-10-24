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
#include <sys/mman.h>

FILE * logfile;

long long tv2usecs(struct timeval *tv)
{
  return tv->tv_sec * 1000000ULL + tv->tv_usec;
}

long long nowusecs(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv2usecs (&tv);
}

int main(){
  unsigned long long x1, x2;
  double total;
  char *attack_args[16];
  int chpid,status;
  int testrounds=1;
  
  attack_args[0]="/home/xcai/project/raceproject/races3flamingo/src/hash-attacker";
  attack_args[1]="0";
  attack_args[2]="0";
  attack_args[3]="filenames.linux4";
  attack_args[4]="/tmp/attack/";
  attack_args[5]="10000";
  attack_args[6]="/tmp/public";
  attack_args[7]="/tmp/secret";
  attack_args[8]="/home/xcai/project/raceproject/races3flamingo/src/victim";
  attack_args[9]="1";
  attack_args[10]="1";
  attack_args[11]="0";
  attack_args[12]="7";
  attack_args[13]="d";
  attack_args[14]="0";
  attack_args[15]=NULL;
  
  
  chpid=fork();
  if(chpid==0){
    execvp("/tmp/clean.sh", NULL);
    perror("execvp00");
       exit(0);
  } else if (chpid == -1) {
    perror("Couldn't fork");
    exit(1);
  } else {
    waitpid(chpid, &status, WUNTRACED | WCONTINUED);
  }
  	
  total=0; 
      
  for(testrounds=1;testrounds<=10;testrounds++){
    x1=nowusecs();
  
  	chpid=fork();
  	if(chpid==0){
    	execvp(attack_args[0], attack_args);
    	perror("execvp11");
        exit(0);
  	} else if (chpid == -1) {
    	perror("Couldn't fork");
    	exit(1);
  	} else {
    	waitpid(chpid, &status, WUNTRACED | WCONTINUED);
  	}
  
    x2=nowusecs();
    total+=(x2-x1);
    
    logfile = fopen ("/home/xcai/flamingologs.c","a");
	fprintf(logfile,"/*********************attack round %d : cost %lf mins********************/\n",testrounds,(double)(x2-x1)/60000000);
	fclose(logfile); 
	    
    chpid=fork();
  	if(chpid==0){
    	execvp("/tmp/clean.sh", NULL);
    	perror("execvp22");
        exit(0);
  	} else if (chpid == -1) {
    	perror("Couldn't fork");
    	exit(1);
  	} else {
    	waitpid(chpid, &status, WUNTRACED | WCONTINUED);
  	}

  }
  
  total/=6000000;
  total/=1000;
  printf("Average time is %lf mins per attack\n",total);
  return 1;
}
