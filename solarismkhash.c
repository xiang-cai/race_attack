#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


unsigned char hexes[64]={'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r' , 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R' , 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '.', '_'};

//OpenSolaris hash function
unsigned int solhash(char *name)
{
  char Xc, *Xcp;
  int hash = 0; //set dvp to 1

  for (Xcp = name; (Xc = *Xcp) != 0; Xcp++)
    hash = (hash << 4) + hash + Xc;
  return hash;
}

struct bday_tag
{
  char name[5];
  int hash;
};

struct bday_tag bday_list[1<<24];

int bday_cmp(const void *va, const void *vb)
{
  struct bday_tag *a = (struct bday_tag *)va;
  struct bday_tag *b = (struct bday_tag *)vb;
  if (a->hash < b->hash)
    return -1;
  if (a->hash > b->hash)
    return 1;
  return 0;
}

int main ()
{
  FILE* fp;
  int i,j,k,l;
  int index = 0;
  fp = fopen("./filenames.solaris","a");

  for(i=0;i<64;i++){
    for(j=0;j<64;j++){
      for(k=0;k<64;k++){
	for(l=0;l<64;l++){
	  bday_list[index].name[0]=hexes[i];
	  bday_list[index].name[1]=hexes[j];
	  bday_list[index].name[2]=hexes[k];
	  bday_list[index].name[3]=hexes[l];
	  bday_list[index].name[4]='\0';
	  bday_list[index++].hash=solhash(bday_list[index].name);
	}
      }
    }
  }
/*
  printf("[0]: %s %d\n",bday_list[0].name,bday_list[0].hash);
  printf("[1]: %s %d\n",bday_list[1].name,bday_list[1].hash);
  printf("[2]: %s %d\n",bday_list[2].name,bday_list[2].hash);
  printf("[3]: %s %d\n",bday_list[3].name,bday_list[3].hash);
*/      
  qsort(bday_list, sizeof(bday_list)/sizeof(bday_list[0]), sizeof(bday_list[0]), bday_cmp);
        
  for (i = 0; i < sizeof(bday_list)/sizeof(bday_list[0]); i++) 
    {
      struct bday_tag key;
      struct bday_tag *match;

      key.hash = -bday_list[i].hash;
      for (j = 0; j < sizeof(key.name)-1; j++)
	key.hash *= 17;
      match = bsearch(&key, bday_list, sizeof(bday_list)/sizeof(bday_list[0]),sizeof(bday_list[0]), bday_cmp);
      while (match && match->hash == key.hash) 
	{
	  char buf[2*sizeof(match->name)];
	  sprintf(buf, "%s%s", bday_list[i].name, match->name);
	  // printf("%s %d\n", buf, solhash(buf));
	  fprintf(fp, "01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789%s\n",buf);
	  match++;
	}

    }

  return 1;
}

