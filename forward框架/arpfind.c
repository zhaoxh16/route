#include "arpfind.h"

int arpGet(struct arpmac *dstmac,char *ifname, struct in_addr *ipStr)  
{  
    struct arpreq req;
    memset(&req, 0, sizeof(struct arpreq));
    printf("ifname:%s\n", ifname);
    printf("ipStr:%s\n",inet_ntoa(*ipStr));

    struct sockaddr_in *sin;
    sin = (struct sockaddr_in*)&req.arp_pa;
    sin->sin_family = AF_INET;
    sin->sin_addr = *ipStr;

    memset(req.arp_dev, 0, 16*sizeof(char));
    strncpy(req.arp_dev, ifname, 16);

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    int ret = ioctl(sock_fd, SIOCGARP, &req);
    if(ret == -1){
        return 0;
    }

    unsigned char *hw = (unsigned char*)req.arp_ha.sa_data;
    printf("%02x-%02x-%02x-%02x-%02x-%02x\n", hw[0],hw[1],hw[2],hw[3],hw[4],hw[5],hw[6]);
    int size = 6;
    unsigned char *dst_mac_head = dstmac->mac;
    while(size>0){
        *dst_mac_head++ = *hw++;
        --size;
    }
    close(sock_fd);
    return 1;  
}  
                                                                                                        
                                                                                                          
                                                                                                            
                                                                                                              
