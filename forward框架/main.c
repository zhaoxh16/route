//#include "analyseip.h"
#include "checksum.h"
#include "lookuproute.h"
#include "arpfind.h"
#include "sendetherip.h"
#include "recvroute.h"
#include <pthread.h>

#define IP_HEADER_LEN sizeof(struct ip)
#define ETHER_HEADER_LEN sizeof(struct ether_header)


//接收路由信息的线程
void *thr_fn(void *arg)
{
	int st=0;
	struct selfroute *selfrt; 
	selfrt = (struct selfroute*)malloc(sizeof(struct selfroute));
	memset(selfrt,0,sizeof(struct selfroute));

	//get if.name
	struct if_nameindex *head, *ifni;
	ifni = if_nameindex();
  	head = ifni;
	char *ifname;

	int sock_fd;
    struct sockaddr_in server_addr;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(800);
    int bidsuc = bind(sock_fd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr));
    int lissuc = listen(sock_fd, 5);
    printf("%d %d\n", bidsuc, lissuc);

	// add-24 del-25
	while(1)
	{
		st=static_route_get(selfrt, sock_fd);
		if(st == 1)
		{
			if(selfrt->cmdnum == 24)
			{
				while(ifni->if_index != 0) {
					if(ifni->if_index==selfrt->ifindex){
						ifname= ifni->if_name;
						break;
					}
					ifni++;
				}
				//插入到路由表里
				insert_route(selfrt->prefix.s_addr, (unsigned int)(selfrt->prefixlen), 
					ifname, selfrt->ifindex, selfrt->nexthop.s_addr);

			}
			else if(selfrt->cmdnum == 25)
			{
				//从路由表里删除路由
				delete_route(selfrt->prefix, (unsigned int)(selfrt->prefixlen));
			}
		}
    }

}

int main()	
{
	char skbuf[1520];
	char data[1480];
	int recvfd,datalen;
	int recvlen;		
	struct ip *ip_recvpkt;
	pthread_t tid;
	ip_recvpkt = (struct ip*)malloc(sizeof(struct ip));

	//创建raw socket套接字
	if((recvfd=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_IP)))==-1)	
	{
		printf("recvfd() error\n");
		return -1;
	}	
	
	//路由表初始化
	routeNodeRoot=(struct RouteNode*)malloc(sizeof(struct RouteNode));
    routeNodeRoot->detail = NULL;
    routeNodeRoot->children[0] = routeNodeRoot->children[1] = NULL;

	//调用添加函数insert_route往路由表里添加直连路由

	//创建线程去接收路由信息
	int pd;
	pd = pthread_create(&tid,NULL,thr_fn,NULL);

	while(1)
	{
		//接收ip数据包模块
		recvlen=recv(recvfd,skbuf,sizeof(skbuf),0);
		int flag = 0;
		if(recvlen>0)
		{
		
			ip_recvpkt = (struct ip *)(skbuf+ETHER_HEADER_LEN);

			//192.168.1.10是测试服务器的IP，现在测试服务器IP是192.168.1.10到192.168.1.80.
			//使用不同的测试服务器要进行修改对应的IP。然后再编译。
			//192.168.6.2是测试时候ping的目的地址。与静态路由相对应。
            //分析打印ip数据包的源和目的ip地址
            int s;
            memset(data,0,1480);
            for(s=0;s<1480;s++)
            {
                data[s]=skbuf[s+34];
            }
            // 校验计算模块
            //调用校验函数check_sum，成功返回1
            unsigned short* iphead_array = (unsigned short*)ip_recvpkt;
            if(!check_sum(iphead_array, sizeof(struct ip)/sizeof(unsigned short))) continue;

            //调用计算校验和函数count_check_sum，返回新的校验和
            unsigned short checksum = count_check_sum(iphead_array);

            //查找路由表，获取下一跳ip地址和出接口模块
            struct nextaddr *nexthopinfo;
            nexthopinfo = (struct nextaddr *)malloc(sizeof(struct nextaddr));
            memset(nexthopinfo,0,sizeof(struct nextaddr));

            //调用查找路由函数lookup_route，获取下一跳ip地址和出接口
            unsigned long* dst_ip = (unsigned long*)(iphead_array+8);
            struct in_addr dstaddr;
            dstaddr.s_addr = *dst_ip;
            if(!lookup_route(dstaddr, nexthopinfo)) {
                continue;
            }
            if (nexthopinfo->ipv4addr.s_addr == 0) {
                nexthopinfo->ipv4addr.s_addr = dstaddr.s_addr;
            }

            //arp find
            struct arpmac *dstmac;
            dstmac = (struct arpmac*)malloc(sizeof(struct arpmac));
            memset(dstmac,0,sizeof(struct arpmac));
            char mac_char[6];
            dstmac->mac = mac_char;

            //调用arpGet获取下一跳的mac地址
            if(arpGet(dstmac, nexthopinfo->ifname, &nexthopinfo->ipv4addr)) {
                ip_transmit(skbuf, nexthopinfo->ifname, dstmac->mac, recvlen);
            }
		}
	}
	close(recvfd);	
	return 0;
}

