#include "arpfind.h"

int arpGet(struct arpmac *dstmac,char *ifname, struct in_addr *ipStr, int sock_fd)
{  
    struct arpreq req;
    memset(&req, 0, sizeof(struct arpreq));

    struct sockaddr_in *sin;
    sin = (struct sockaddr_in*)&req.arp_pa;
    sin->sin_family = AF_INET;
    sin->sin_addr = *ipStr;

    strncpy(req.arp_dev, ifname, 16);

    int ret = ioctl(sock_fd, SIOCGARP, &req);
    if(ret == -1){
        return 0;
    }
    memcpy(dstmac->mac, req.arp_ha.sa_data, 6);
    return 1;
}  
                                                                                                        
                                                                                                          
                                                                                                            
                                                                                                              
