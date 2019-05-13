#include "lookuproute.h"


int insert_route(unsigned long ip4prefix, unsigned int prefixlen, int num){
    struct RouteNode* routeNodeNow = routeNodeRoot;
    unsigned int prefixlenNow = prefixlen;
    unsigned long ip4prefixNow = ip4prefix;
    unsigned long mask = (1 << 31);
    while(prefixlenNow > 0){
        prefixlenNow -= 1;
        int bitNum;
        if(ip4prefixNow & mask) bitNum = 1;
        else bitNum = 0;
        printf("bitNum: %d\n", bitNum);
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
    if(routeNodeNow->detail != NULL){
        free(routeNodeNow->detail);
    }
    routeNodeNow->detail = (struct route*)malloc(sizeof(struct route));
    struct route* routeDetail = routeNodeNow->detail;
    routeDetail->num = num;
}

int lookup_route(unsigned long ip4addr, struct route* numStruct){
    struct RouteNode* routeNodeNow = routeNodeRoot;
    struct route* rightRoute = NULL;
    unsigned long ip4prefixNow = ip4addr;
    unsigned long mask = 1 << 31;
    int numPos;
    if(ip4prefixNow & mask) numPos = 1;
    else numPos = 0;
    while(routeNodeNow->children[numPos] != NULL){
        routeNodeNow = routeNodeNow->children[numPos];
        if(routeNodeNow->detail != NULL){
            rightRoute = routeNodeNow->detail;
        }
        ip4prefixNow = ip4prefixNow << 1;
        if(ip4prefixNow & mask) numPos = 1;
        else numPos = 0;
    }
    if(rightRoute == NULL) return 0;
    numStruct->num = rightRoute->num;
    return 1;
}
