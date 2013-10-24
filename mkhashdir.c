#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/param.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

char hexes[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
									 '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

/* Just generate names randomly by permuting the last 16
	 bytes.  The linux name hash is invariant under character
	 permutation, and the odds of getting the same name twice 
	 randomly is negligiable. 

   The linux hash function is not permutation invariant -- we misread
	 the source code. -- rob */
void nextname_perm(unsigned char *name, int len)
{
	int i;
	unsigned char t;
	unsigned char *suffix = name + len - 17;

	for (i = 0; i < 15; i++) {
		int j = rand() % (16 - i);		
		t = suffix[i];
		suffix[i] = suffix[i+j];
		suffix[i+j] = t;
	}
}

/* Some filenames with hash == 0 under linux. */
char * names[] = 
	{ "bvttkgtfbvttkgtfbvttkgtf",
    "bvttkgtfbvttkgtfbvuhuzgf",
    "bvttkgtfbvttkgtfbvuiknys",
    "bvttkgtfbvttkgtfbvuilcys",
    "bvttkgtfbvttkgtfbvujacys",
    "bvttkgtfbvttkgtfbwjirttf",
    "bvttkgtfbvttkgtfckupqils",
    "bvttkgtfbvttkgtfckupqjas",
    "bvttkgtfbvttkgtfckuqettf",
    "bvttkgtfbvttkgtfckvffpys",
    "bvttkgtfbvttkgtfcljpxnys",
    "bvttkgtfbvttkgtfcljpycys",
    "bvttkgtfbvttkgtfclkfnmgf",
    "bvttkgtfbvttkgtfclkfobgf",
		"bvttkgtfbvttkgtfclkfnmgf"
    "bvttkgtfbvttkgtfclkfobgf",
	};

/******************
 * The linux filename hash function.
 ******************/

/* Name hashing routines. Initial hash value */
/* Hash courtesy of the R5 hash in reiserfs modulo sign bits */
#define init_name_hash()                0

/* partial hash update function. Assume roughly 4 bits per character */
static inline unsigned long
partial_name_hash(unsigned long c, unsigned long prevhash)
{
	return (prevhash + (c << 4) + (c >> 4)) * 11;
}

/*
 * Finally: cut down the number of bits to a int value (and try to avoid
 * losing bits)
 */
static inline unsigned long end_name_hash(unsigned long hash)
{
	return (unsigned int) hash;
}

/* Compute the hash for a name string. */
static inline unsigned int
full_name_hash(const unsigned char *name, unsigned int len)
{
	unsigned long hash = init_name_hash();
	while (len--)
		hash = partial_name_hash(*name++, hash);
	return end_name_hash(hash);
}

/***************
 * A birthday attack on the linux filename hash function.
 ***************/

struct bday_entry {
	unsigned char name[6];
	unsigned int hash;
} bday_list[1<<25];

int bday_cmp(const void *va, const void *vb)
{
	struct bday_entry *a = (struct bday_entry *)va;
	struct bday_entry *b = (struct bday_entry *)vb;
	if (a->hash < b->hash)
		return -1;
	if (a->hash > b->hash)
		return 1;
	return 0;
}

void gen_rand_name(struct bday_entry *be)
{
	int i;
	for (i = 0; i < sizeof(be->name); i++) {
		be->name[i] = rand() % 64;
		if (be->name[i] < 10) be->name[i] += '0';
		else if (be->name[i] < 36) be->name[i] += 'A' - 10;
		else if (be->name[i] < 62) be->name[i] += 'a' - 36;
		else if (be->name[i] == 62) be->name[i] = '_';
		else if (be->name[i] == 63) be->name[i] = '-';
	}
}

void build_bday_list(void)
{
	int i, j;

	for (i = 0; i < sizeof(bday_list) / sizeof(bday_list[0]); i++) {
		gen_rand_name(&bday_list[i]);
		bday_list[i].hash = full_name_hash(bday_list[i].name, 
																			 sizeof(bday_list[i].name));
	}
	qsort(bday_list, sizeof(bday_list)/sizeof(bday_list[0]),
				sizeof(bday_list[0]), bday_cmp);

	for (i = 0; i < sizeof(bday_list)/sizeof(bday_list[0]); i++) {
		struct bday_entry key;
		struct bday_entry *match;

		key.hash = -bday_list[i].hash;
		for (j = 0; j < sizeof(key.name); j++)
			key.hash *= 11;
		match = bsearch(&key, bday_list, sizeof(bday_list)/sizeof(bday_list[0]),
										sizeof(bday_list[0]), bday_cmp);
		while (match && match->hash == key.hash) {
			unsigned char buf[2*sizeof(match->name)+1];
			memcpy(buf, bday_list[i].name, sizeof(bday_list[i].name));
			memcpy(buf + sizeof(bday_list[i].name), 
						 match->name, sizeof(match->name));
			buf[sizeof(buf)-1] = '\0';
			printf("%s\n", buf);
			match++;
		}
	}
}

unsigned char SN(unsigned char x)
{
	return (x << 4) + (x >> 4);
}

int isok(unsigned char c)
{ 
	return c != '\0' && c != '/';
}

void nextname_hash(unsigned char *name, int len)
{
	unsigned int oldhash, newhash;
	int i;

	oldhash = full_name_hash(name, len-1);
	i = len - 3;
	while (i >= 0 && 
				 (!isok(SN(SN(name[i]) - 1)) || !isok(SN(SN(name[i+1]) + 11))))
		i--;
	if (i >= 0) {
		name[i] = SN(SN(name[i]) - 1);
		name[i+1] = SN(SN(name[i+1]) + 11);
	}
	newhash = full_name_hash(name, len-1);
	printf("%d %u %u\n", strlen((char *)name), oldhash, newhash);
}

void nextname_rand(unsigned char *name, int len)
{
	int i;
	unsigned char *suffix = name + len - 17;

	for (i = 0; i < 16; i++) {
		suffix[i] = hexes[rand() % 16];
	}
}

unsigned char *filenames[1<<17];

void nextname_file_list_init(char *arg)
{
	FILE *f;
	int i;
	
	f = fopen(arg, "r");
	i = 0;
	while(fscanf(f, "%as\n", &filenames[i]) != EOF)
		i++;
}

void nextname_file_list(unsigned char *name, int len)
{
	static int index = 0;
	int flen;

	if (filenames[index] == NULL)
		index = 0;

	flen = strlen((char *)filenames[index]);
	memcpy(name + len - flen - 1, filenames[index], flen);
	index++;
}


void nextname_linux_init(char *arg)
{

	nextname_file_list_init("filenames.linux");
}

struct { 
	char mode; 
	void (*nextname_init)(char *);
	void (*nextname)(unsigned char *, int); } modes[] =
		{{ 'p', NULL,                nextname_perm },
		 { 'r', NULL,                nextname_rand },
		 { 'f', nextname_file_list_init, nextname_file_list},
		 { 'l', nextname_linux_init, nextname_file_list } };

void nextname_init(char *mode)
{
	int i;
	for (i = 0; i < sizeof(modes) / sizeof(modes[0]); i++)
		if (modes[i].mode == mode[0] && modes[i].nextname_init)
			return modes[i].nextname_init(mode + 2);
}

void nextname(char *mode, unsigned char *name, int len)
{
	int i;
	for (i = 0; i < sizeof(modes) / sizeof(modes[0]); i++)
		if (modes[i].mode == mode[0])
			return modes[i].nextname(name, len);
}

long long tv2usecs(struct timeval *tv)
{
	return tv->tv_sec * 1000000ULL + tv->tv_usec;
}

int main(int argc, char **argv)
{
	char *mode;
	int nfiles;
	unsigned int len;
	char *dir;
	unsigned char *basename;
	int baselen;
	char fname[256];
	int i;
	char first_fname[256];
	char last_fname[256];
	struct timeval tv[8];

	srand(time(NULL));

	if (argc == 2 && strcmp(argv[1], "-b") == 0) {
		build_bday_list();
		exit(0);
	} else if (argc != 6) {
		printf("%s <mode> <nfiles> <dir> <basename>\n"
					 "  mode:     r: random filenames\n"
           "            p: permuted filenames\n"
					 "            l: linux hash-invariant filenames\n"
					 "            f:list-file load filenames from list-file\n"
					 "  nfiles:   number of files to create\n"
					 "  len:      length of filenames\n"
					 "  dir:      location to create files\n"
					 "  basename: basename of created files\n",
					 argv[0]);
		exit(0);
	}

	mode = argv[1];
	nfiles = atoi(argv[2]);
	len = atoi(argv[3]);
	if (len > 255) len = 255;
	dir = argv[4];
	basename = (unsigned char *)argv[5];
	
	baselen = snprintf(fname, sizeof(fname), "%s.", basename);
	for (i = baselen; i < len; i++)
		fname[i] = hexes[i % 16];
	fname[len] = '\0';
	srand(time(NULL));

	nextname_init(mode);

	if (chdir(dir)) {
		perror("Cannot change to target directory\n");
		exit(0);
	}

	/* 	for (i = 0; i < sizeof(names)/sizeof(names[0]); i++) */
	/* 		printf("%u\n", */
	/* 					 full_name_hash((unsigned char *)names[i], strlen(names[i]))); */

	gettimeofday(&tv[0], NULL);
	for (i = 0; i < nfiles; i++) {
		nextname(mode, (unsigned char *)fname, len+1);
		if (i == 0)
			memcpy(first_fname, fname, len+1);
		//printf("%s\n", fname);
		close(creat(fname, S_IRWXU | S_IRWXG | S_IRWXO));
	}
	memcpy(last_fname, fname, len+1);
	nextname(mode, (unsigned char *)fname, len+1);

#define NTRIALS 10000

	gettimeofday(&tv[1], NULL);
	for (i = 0; i < NTRIALS; i++)
		close(open(first_fname, O_RDONLY));
	gettimeofday(&tv[2], NULL);
	for (i = 0; i < NTRIALS; i++)
		close(open(last_fname, O_RDONLY));
	gettimeofday(&tv[3], NULL);
	for (i = 0; i < NTRIALS; i++)
		close(open(fname, O_RDONLY));
	gettimeofday(&tv[4], NULL);
	for (i = 0; i < NTRIALS; i++)
		close(open(first_fname, O_RDONLY));
	gettimeofday(&tv[5], NULL);
	for (i = 0; i < NTRIALS; i++)
		close(open(last_fname, O_RDONLY));
	gettimeofday(&tv[6], NULL);
	for (i = 0; i < NTRIALS; i++)
		close(open(fname, O_RDONLY));
	gettimeofday(&tv[7], NULL);

	printf("%8lld\n%8lld %8lld %8lld\n%8lld %8lld %8lld\n", 
				 tv2usecs(&tv[1]) - tv2usecs(&tv[0]),
				 tv2usecs(&tv[2]) - tv2usecs(&tv[1]),
				 tv2usecs(&tv[3]) - tv2usecs(&tv[2]),
				 tv2usecs(&tv[4]) - tv2usecs(&tv[3]),
				 tv2usecs(&tv[5]) - tv2usecs(&tv[4]),
				 tv2usecs(&tv[6]) - tv2usecs(&tv[5]),
				 tv2usecs(&tv[7]) - tv2usecs(&tv[6]));

	return 0;
}

