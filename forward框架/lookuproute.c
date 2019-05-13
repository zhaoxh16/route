#include "lookuproute.h"


int insert_route(unsigned long ip4prefix, unsigned int prefixlen, char *ifname, unsigned int ifindex, unsigned long nexthopaddr){
    // find and insert route node
    struct RouteNode* routeNodeNow = routeNodeRoot;
    unsigned int prefixlenNow = prefixlen;
    unsigned long ip4prefixNow = ntohl(ip4prefix);
    unsigned long mask = (1 << 31);
    while(prefixlenNow > 0){
        prefixlenNow -= 1;
        int bitNum = ip4prefixNow & mask;
        ip4prefixNow = ip4prefixNow << 1;
        if(routeNodeNow->children[bitNum] == NULL){
            routeNodeNow->children[bitNum] = (struct RouteNode*)malloc(sizeof(struct RouteNode));
            routeNodeNow = routeNodeNow->children[bitNum];
            routeNodeNow->children[0] = routeNodeNow->children[1] = routeNodeNow->detail = NULL;
        }
        else{
            routeNodeNow = routeNodeNow->children[bitNum];
        }
    }
    // add detail to route
    routeNodeNow->detail = (struct route*)malloc(sizeof(struct route));
    struct route* routeDetail = routeNodeNow->detail;
    routeDetail->prefixlen = prefixlen;
    routeDetail->nexthop = (struct nexthop*)malloc(sizeof(struct nexthop));
    routeDetail->nexthop->ifname = ifname;
    routeDetail->nexthop->ifindex = ifindex;
    memcpy(&routeDetail->ip4prefix, &ip4prefix, sizeof(struct in_addr));
    memcpy(&routeDetail->nexthop->nexthopaddr, &nexthopaddr, sizeof(struct in_addr));
}

int lookup_route(struct in_addr dstaddr, struct nextaddr *nexthopinfo){
    struct RouteNode* routeNodeNow = routeNodeRoot;
    struct route* rightRoute = NULL;
    unsigned long ip4prefixNow = ntohl(dstaddr.s_addr);
    unsigned long mask = 1 << 31;
    int numPos = ip4prefixNow & mask;
    while(routeNodeNow->children[numPos] != NULL){
        routeNodeNow = routeNodeNow->children[numPos];
        if(routeNodeNow->detail != NULL){
            rightRoute = routeNodeNow->detail;
        }
        ip4prefixNow = ip4prefixNow << 1;
        numPos = ip4prefixNow & mask;
    }
    if(rightRoute == NULL) return 0;
    nexthopinfo->ifname = rightRoute->nexthop->ifname;
    nexthopinfo->ipv4addr = rightRoute->nexthop->nexthopaddr;
    nexthopinfo->prefixl = rightRoute->prefixlen;
    return 1;
}

int delete_route(struct in_addr dstaddr, unsigned int prefixlen){
    unsigned long ip4prefixNow = ntohl(dstaddr.s_addr);
    unsigned long mask = 1 << 31;
    int numPos = ip4prefixNow & mask;
    unsigned int prefixlenNow = prefixlen;
    struct RouteNode* routeNodeNow = routeNodeRoot;
    while(prefixlenNow != 0 && routeNodeNow->children[numPos] != NULL){
        routeNodeNow = routeNodeNow->children[numPos];
        prefixlenNow -= 1;
        ip4prefixNow = ip4prefixNow << 1;
        numPos = ip4prefixNow & mask;
    }
    if(prefixlenNow == 0){
        if(routeNodeNow->detail != NULL){
            struct route* detail = routeNodeNow->detail;
            routeNodeNow->detail = NULL;
            free(detail->nexthop);
            free(detail);
            return 1;
        }
        else return 0;
    }
    else return 0;
}