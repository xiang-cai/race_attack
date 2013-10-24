#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


//OpenSolaris hash function
unsigned int solhash(char *name)
{
  char Xc, *Xcp;
  int hash = 5381; //set dvp to 1

  for (Xcp = name; *Xcp && (Xc = *Xcp) != '/'; Xcp++)
    hash = (hash << 5) + hash + Xc;
  //hash = hash % 32;
  return hash;
}


int main ()
{
  char* buf1="0123456789012345678901.Dyer2Yi/";
  char* buf2="01234567890123456789010EjCCQK5/";
  printf("%d\n%d\n", solhash(buf1),solhash(buf2));

  return 1;
}

