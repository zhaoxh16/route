#include "checksum.h"

int check_sum(unsigned short *iphd,int len)
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
