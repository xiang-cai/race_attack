#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>
#include <stdint.h>

unsigned char hexes[64]={'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r' , 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R' , 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '.', '_'};

int main(){
	int i,j,k;
	FILE* out;

	out = fopen("./fnames","a");
	for(i=0;i<10;i++){
		for(j=0;j<10;j++){
			for(k=0;k<10;k++){
				fprintf(out, "%c%c%c\n",hexes[i],hexes[j],hexes[k]);
			}
		}
	}
	fclose(out);
	return 0;
}
