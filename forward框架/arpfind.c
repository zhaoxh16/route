#include "arpfind.h"

int arpGet(struct arpmac *dstmac,char *ifname, struct in_addr *ipStr)  
{  
    struct arpreq req;
    memset(&req, 0, sizeof(struct arpreq));

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
    int size = 6;
    unsigned char *dst_mac_head = dstmac->mac;
    while(size>0){
        *dst_mac_head++ = *hw++;
        --size;
    }
    close(sock_fd);
    return 1;  
}  
                                                                                                        
                                                                                                          
                                                                                                            
                                                                                                              
