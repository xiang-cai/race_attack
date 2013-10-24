#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//#define INIT ((unsigned int)33554467UL)
#define INIT ((unsigned int)916886723UL)
#define PRIME ((unsigned int)0x01000193UL)
#define INVERSE ((unsigned int)899433627UL)


unsigned char hexes[64]={'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r' , 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R' , 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '.', '_'};

typedef struct _tab{
	unsigned int hval;
	char* ptr;
}tab;

char* hashnames[1<<20];

unsigned int left_one[1<<6][2];
unsigned int left_two[1<<12][3];
unsigned int left_three[1<<18][4];
tab left_four[1<<24];

unsigned int right_three[1<<18][4];
unsigned int right_two[1<<12][3];
unsigned int right_one[1<<6][2];
tab right_four[1<<24];


unsigned int inverse_hash_one_hex(unsigned int hval,unsigned char hex){
    hval^=hex;
	hval*=INVERSE;
    return hval;
}

unsigned int hash_one_hex(unsigned int hval,unsigned char hex){
		hval *= PRIME;
		hval ^= hex;
		return hval;
}

void dohash_left (){
	int i,tab_index,hex_index;

	for(i=0;i<(1<<6);i++){
		left_one[i][1]=hexes[i];
		left_one[i][0]=hash_one_hex(INIT,hexes[i]);
	}

	tab_index=0;
	for(i=0;i<(1<<6);i++){
		for(hex_index=0;hex_index<64;hex_index++){
			left_two[tab_index][1]=left_one[i][1];
			left_two[tab_index][2]=hexes[hex_index];
			left_two[tab_index][0]=hash_one_hex(left_one[i][0],hexes[hex_index]);
			tab_index++;
		}
	}

	tab_index=0;
	for(i=0;i<(1<<12);i++){
		for(hex_index=0;hex_index<64;hex_index++){
			left_three[tab_index][1]=left_two[i][1];
			left_three[tab_index][2]=left_two[i][2];
			left_three[tab_index][3]=hexes[hex_index];
		    left_three[tab_index][0]=hash_one_hex(left_two[i][0],hexes[hex_index]);
			tab_index++;
		}
	}

	tab_index=0;
	for(i=0;i<(1<<18);i++){
		for(hex_index=0;hex_index<64;hex_index++){
            left_four[tab_index].ptr=(char*)malloc(4*sizeof(char));
			left_four[tab_index].ptr[0]=left_three[i][1];
			left_four[tab_index].ptr[1]=left_three[i][2];
			left_four[tab_index].ptr[2]=left_three[i][3];
			left_four[tab_index].ptr[3]=hexes[hex_index];
		    left_four[tab_index].hval=hash_one_hex(left_three[i][0],hexes[hex_index]);
			tab_index++;
		}
	}

}

void dohash_right(){
	int i,tab_index,hex_index;

	for(i=0;i<(1<<6);i++){
		right_one[i][1]=hexes[i];
		right_one[i][0]=inverse_hash_one_hex(0,hexes[i]);
	}

	tab_index=0;
	for(i=0;i<(1<<6);i++){
		for(hex_index=0;hex_index<64;hex_index++){
			right_two[tab_index][2]=left_one[i][1];
			right_two[tab_index][1]=hexes[hex_index];
			right_two[tab_index][0]=inverse_hash_one_hex(right_one[i][0],hexes[hex_index]);
			tab_index++;
		}
	}

	tab_index=0;
	for(i=0;i<(1<<12);i++){
		for(hex_index=0;hex_index<64;hex_index++){
			right_three[tab_index][3]=right_two[i][2];
			right_three[tab_index][2]=right_two[i][1];
			right_three[tab_index][1]=hexes[hex_index];
		    right_three[tab_index][0]=inverse_hash_one_hex(right_two[i][0],hexes[hex_index]);
			tab_index++;
		}
	}

	tab_index=0;
	for(i=0;i<(1<<18);i++){
		for(hex_index=0;hex_index<64;hex_index++){
            right_four[tab_index].ptr=(char*)malloc(4*sizeof(char));
			right_four[tab_index].ptr[3]=right_three[i][3];
			right_four[tab_index].ptr[2]=right_three[i][2];
			right_four[tab_index].ptr[1]=right_three[i][1];
			right_four[tab_index].ptr[0]=hexes[hex_index];
		    right_four[tab_index].hval=inverse_hash_one_hex(right_three[i][0],hexes[hex_index]);
			tab_index++;
		}
	}

}


static int cmpr(const void *p1, const void *p2){
	const tab* ptr1=(const tab*)p1;
	const tab* ptr2=(const tab*)p2;

	if(ptr1->hval<ptr2->hval)
		return -1;
	else if(ptr1->hval==ptr2->hval)
		return 0;
	else
		return 1;
}

unsigned int findnames(){
	unsigned int count=0;
	unsigned int nameindex=0;
	unsigned int i,j;
	unsigned int la,lb;
	unsigned int ra,rb;
	unsigned int inl,inr;
	i=j=0;

	for(;i<(1<<24);){
		for(;j<(1<<24);){
			if(left_four[i].hval>right_four[j].hval){
				j++;
				continue;
			}
			else if(left_four[i].hval<right_four[j].hval){
				i++;
				break;
			}
			else{
				la=lb=i;
				ra=rb=j;
				while(lb<(1<<24) && left_four[lb].hval==left_four[la].hval)
					lb++;
				lb--;
				while(rb<(1<<24) && right_four[rb].hval==right_four[ra].hval)
					rb++;
				rb--;
				for(inl=la;inl<=lb;inl++)
					for(inr=ra;inr<=rb;inr++){
						hashnames[nameindex]=(char*)malloc(8*sizeof(char));
						hashnames[nameindex][0]=left_four[inl].ptr[0];
						hashnames[nameindex][1]=left_four[inl].ptr[1];
						hashnames[nameindex][2]=left_four[inl].ptr[2];
						hashnames[nameindex][3]=left_four[inl].ptr[3];
						hashnames[nameindex][4]=right_four[inr].ptr[0];
						hashnames[nameindex][5]=right_four[inr].ptr[1];
						hashnames[nameindex][6]=right_four[inr].ptr[2];
						hashnames[nameindex][7]=right_four[inr].ptr[3];
						nameindex++;
					}
				i=lb+1;
				j=rb+1;
				count++;
			}
		}
	}
	return count;
}

int main(){
	FILE* pfile;
	unsigned int i;
	unsigned int num=0;

	dohash_left();
	dohash_right();
	qsort(left_four, (1<<24), sizeof(tab),cmpr);
	qsort(right_four, (1<<24), sizeof(tab),cmpr);

	num=findnames();
//	printf("%u names found\n",num);

	pfile=fopen("./filenames.freebsd","a");
	for(i=0;i<num;i++){
		fprintf(pfile,"012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789%c%c%c%c%c%c%c%c\n",hashnames[i][0],hashnames[i][1],hashnames[i][2],hashnames[i][3],hashnames[i][4],hashnames[i][5],hashnames[i][6],hashnames[i][7]);
	}
	fclose(pfile);
	return 1;
}
