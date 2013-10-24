#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>


int main(){

	long i;
	int rounds;

	long touch;
	
	long size=1;
	
	for(i=1;i<=28;i++)
		size*=2;


	unsigned long long* larray=(unsigned long long *)malloc(size*sizeof(unsigned long long));
		
//	for(rounds=1;rounds<=3;rounds++){		


	
	
		srand (time(NULL)*time(NULL)*time(NULL)*time(NULL)*time(NULL)*time(NULL)*time(NULL)*time(NULL)*time(NULL)*time(NULL)*time(NULL)*time(NULL));
	
		for(i=1;i<=1000000;i++){
				
			touch = rand()%size;
		
			larray[touch]=touch;		

		}
	
		free(larray);
	
//	}
	
	return 1;
}
