#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

long long tv2usecs(struct timeval *tv)
{
	return tv->tv_sec * 1000000ULL + tv->tv_usec;
}

int main(int argc, char **argv)
{
	int dofork = 0;
	int pid = 0;
	int i;
	int trials;
	char *fname;
	struct timeval tv[4];
	int fd;
	char buf[100];
	unsigned long long mintime = -1ULL;

	if (strcmp(argv[1], "-f") == 0) {
		if ((pid = fork()) == -1) {
			perror("Couldn't fork");
			exit(1);
		}
		argv++;
		argc--;
		dofork = 1;
	}

	trials = atoi(argv[1]);
	fname = argv[2];

	gettimeofday(&tv[0], NULL);
	for (i = 0; i < trials; i++) {
		unsigned long long newtime;
		gettimeofday(&tv[1], NULL);
		fd = open(fname, O_RDONLY);
		gettimeofday(&tv[2], NULL);
		if (fd < 0) {
			perror("Couldn't open file");
			exit(2);
		}
		newtime = tv2usecs(&tv[2]) - tv2usecs(&tv[1]);
		if (newtime < mintime)
			mintime = newtime;

		if (dofork) {
			// Don't use printf so the write is atomic
			sprintf(buf, "%d %lld %lld\n", pid, tv2usecs(&tv[1]), newtime); 
			write(1, buf, strlen(buf));
		}
		close(fd);
	}
	gettimeofday(&tv[3], NULL);
	printf("%d %lld %lld %f\n", 
				 pid, 
				 tv2usecs(&tv[3]) - tv2usecs(&tv[0]), 
				 mintime,
				 ((double)tv2usecs(&tv[3]) - tv2usecs(&tv[0]))/trials);

	if (pid)
		wait(&i);

	return 0;
}
