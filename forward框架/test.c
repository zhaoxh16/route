#include <stdio.h>
#include <stdlib.h>
int check_sum(unsigned short *iphd,int len,unsigned short checksum)
{
    unsigned int ckSum = 0;
    int my_len = 10;
    while(my_len>0){
        my_len-=1;
        ckSum += *iphd++;
    }
    ckSum = (ckSum>>16)+(ckSum&0xFFFF);
    ckSum += (ckSum>>16);
    unsigned short shortCkSum = (unsigned short)(~ckSum);
    if(shortCkSum == 0) return 1;
    else return 0;
}
unsigned short count_check_sum(unsigned short *iphd)
{
    unsigned char* ttl = (unsigned char*)(iphd+4);
    *ttl -= 1;
    unsigned short* ck = iphd + 5;
    *ck += 1;
    return *ck;
}

int main(){
    unsigned short input[10];
    input[0] = 0x4500;
    input[1] = 0x0031;
    input[2] = 0x89F5;
    input[3] = 0x0000;
    input[4] = 0x6e06;
    input[5] = 0xDD38;
    input[6] = 0xDEB7;
    input[7] = 0x455D;
    input[8] = 0xC0A8;
    input[9] = 0x00DC;
    int result = check_sum(input, 10, input[5]);
    printf("result:%d\n", result);
    unsigned short ck = count_check_sum(input);
    printf("check sum:%4x\n", ck);
    for(int i=0;i<10;++i){
        printf("input[%d]:%4x\n", i, input[i]);
    }
}

