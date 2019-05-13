#include <stdio.h>
#include "lookuproute.h"

int main() {
    routeNodeRoot = (struct RouteNode*)malloc(sizeof(struct RouteNode));

    unsigned long ip4prefix = 0b1111 << 28;
    printf("%x\n", ip4prefix);
    insert_route(ip4prefix, 4, 1);
    struct route myRoute;
    myRoute.num = 0;
    unsigned long ip4addr = 0b111111 << 26;
    lookup_route(ip4addr, &myRoute);
    printf("%d\n", myRoute.num);
    return 0;
}