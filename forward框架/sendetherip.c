#include "sendetherip.h"

/* ip transmit */
void ip_transmit(char* skbuf, char *ifName,unsigned char *nextmac, int len)
{
    printf("len:%d\n", len);

    int sockfd;
    if((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1){
        perror("socket");
    }

    struct ifreq if_mac;
    struct ifreq if_idx;

	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
	    perror("SIOCGIFINDEX");

    memset(&if_mac, 0, sizeof(struct ifreq));
    strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
    if(ioctl(sockfd, SIOCGIFHWADDR, &if_mac)<0)
        perror("SIOCGIFHWADDR");
    
    unsigned char* my_mac = if_mac.ifr_hwaddr.sa_data;
    printf("My mac:%02x-%02x-%02x-%02x-%02x-%02x\n", my_mac[0], my_mac[1], 
        my_mac[2], my_mac[3], my_mac[4], my_mac[5]);
    printf("Next mac:%02x-%02x-%02x-%02x-%02x-%02x\n", nextmac[0], nextmac[1], nextmac[2], nextmac[3], nextmac[4], nextmac[5]);

    struct ether_header* eh = (struct ether_header*)skbuf;
    eh->ether_type = htons(ETHERTYPE_IP);
    memcpy(skbuf, nextmac, 6);
    memcpy(skbuf+6, if_mac.ifr_hwaddr.sa_data, 6);

    unsigned int ifindex = if_nametoindex(ifName);
    printf("ifindex:%d",if_idx.ifr_ifindex);
    struct sockaddr_ll socket_address;
    socket_address.sll_ifindex = if_idx.ifr_ifindex;
    socket_address.sll_halen = ETH_ALEN;
    memcpy(socket_address.sll_addr, nextmac, 6);
    printf("finish construct packet\n");


    if(sendto(sockfd, skbuf, len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll))<0)
        printf("send failed\n");
    printf("send finished\n");


    unsigned char* ccc = skbuf;
    int ttt=0;
    for(ttt=0;ttt<len;++ttt){
        printf("%02x ",*(ccc+ttt));
    }
    printf("\n1 end\n");

    return;
}
