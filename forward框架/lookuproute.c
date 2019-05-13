#include "lookuproute.h"
#include <stdio.h>


int insert_route(unsigned long ip4prefix,unsigned int prefixlen,char *ifname,unsigned int ifindex,unsigned long  nexthopaddr)
{
    route_table->prefixlen = prefixlen;
    route_table->nexthop = (struct nexthop*)malloc(sizeof(struct nexthop));
    route_table->nexthop->ifname = ifname;
    route_table->nexthop->ifindex = ifindex;
    memcpy(&route_table->ip4prefix, &ip4prefix, sizeof(struct in_addr));
    memcpy(&route_table->nexthop->nexthopaddr, &nexthopaddr, sizeof(struct in_addr));
    struct route* route_table_next = route_table;
    route_table = (struct route*)malloc(sizeof(struct route));
    memset(route_table, 0, sizeof(struct route));
    route_table->next = route_table_next;

    // print
    struct route * route_head = route_table;
    while(route_head->next != NULL){
        printf("%s\n", inet_ntoa(route_head->ip4prefix));
        if(route_head->nexthop != NULL){
            printf("%s\n", inet_ntoa(route_head->nexthop->nexthopaddr));
        }
        route_head = route_head->next;
    }
}

int lookup_route(struct in_addr dstaddr,struct nextaddr *nexthopinfo)
{
    struct route* route_head = route_table;
    char flag = 0;
    struct route* right_route = NULL;
    int max_prefix_len = 0;
    while(route_head->next != NULL){
        route_head = route_head->next;
        unsigned int mask = 0xFFFFFFFF>>(32-route_head->prefixlen);
        if((route_head->ip4prefix.s_addr & mask) == (dstaddr.s_addr & mask)){
            if(route_head->prefixlen>max_prefix_len){
                flag = 1;
                max_prefix_len = route_head->prefixlen;
                right_route = route_head;
            }
        }
    }
    if(!flag) return 0;
    nexthopinfo->ifname = right_route->nexthop->ifname;
    nexthopinfo->ipv4addr = right_route->nexthop->nexthopaddr;
    nexthopinfo->prefixl = right_route->prefixlen;
    return 1;
}

int delete_route(struct in_addr dstaddr,unsigned int prefixlen)
{
	unsigned int mask = 0xFFFFFFFF>>(32-prefixlen);
    unsigned int ip4prefix = dstaddr.s_addr & mask;
    struct route* route_head = route_table;
    char flag = 0;
    while(route_head->next != NULL){
        struct route* route_before = route_head;
        route_head = route_head->next;
        if(route_head->prefixlen == prefixlen){
            unsigned int route_ip4prefix = route_head->ip4prefix.s_addr & mask;
            if(route_ip4prefix == ip4prefix){
                route_before->next = route_head->next;
                free(route_head->nexthop);
                free(route_head);
                flag = 1;
                break;
            }
        }
    }
    return flag;
}

