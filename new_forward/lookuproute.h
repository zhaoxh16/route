#ifndef __FIND__
#define __FIND__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


struct RouteNode{
    struct route* detail;
    struct RouteNode* children[2];
};

struct route{
    struct in_addr ip4prefix;
    unsgined int prefixlen;
    struct nexthop *nexthop;
};

struct nexthop
{
   char *ifname;
   unsigned int ifindex;
   struct in_addr nexthopaddr;
};

struct nextaddr
{
   char *ifname;
   struct in_addr ipv4addr;
   unsigned int prefixl;
};


struct RouteNode* routeNodeRoot;

int insert_route(unsigned long ip4prefix, unsigned int prefixlen, char *ifname, unsigned int ifindex, unsigned long nexthopaddr);
int lookup_route(struct in_addr dstaddr,struct nextaddr *nexthopinfo);
int delete_route(struct in_addr dstaddr,unsigned int prefixlen);
