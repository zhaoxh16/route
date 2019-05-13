#include <stdio.h>
#include "lookuproute.h"

int main() {
    printf("Hello, World!\n");
    routeNodeRoot = (struct RouteNode*)malloc(sizeof(struct RouteNode));
    struct nextaddr nexthopinfo;
    nexthopinfo.ifname = NULL;
    struct in_addr in1;
    unsigned long ip4prefix = inet_aton("192.168.100.1", &in1);
    unsigned int prefixlen = 24;
    char *ifname = "test";
    unsigned int ifindex = 0;
    struct in_addr in2;
    unsigned long nexthopaddr = inet_aton("192.168.101.1", &in2);
    int result = insert_route(in1.s_addr, prefixlen, ifname, ifindex, in2.s_addr);
    printf("insert result: %d\n", result);
    struct in_addr lookup_struct;
    inet_aton("192.168.100.1", &lookup_struct);
    int result2 = lookup_route(lookup_struct, &nexthopinfo);
    printf("lookup result: %d\n", result2);
    printf("ifname: %s\n", nexthopinfo.ifname);
    printf("addr: %s\n", inet_ntoa(nexthopinfo.ipv4addr));
    printf("prefixlen: %d\n", nexthopinfo.prefixl);
    return 0;
}