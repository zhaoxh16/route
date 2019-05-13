#ifndef __FIND__
#define __FIND__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct RouteNode{
    struct route* detail;
    struct RouteNode* children[2];
};

struct route{
    int num;
};

struct RouteNode* routeNodeRoot;

int insert_route(unsigned long ip4prefix, unsigned int prefixlen, int num);
int lookup_route(unsigned long ip4addr,struct route* numStruct);
#endif