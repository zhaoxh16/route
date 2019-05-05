#include "rip.h"

TRtEntry *g_pstRouteEntry = NULL;

struct in_addr pcLocalAddr[10];//存储本地接口ip地址
char *pcLocalName[10]={};//存储本地接口的接口名
struct in_addr pcLocalMask[10];

int pcLocalInterfaceNum;
TRipPkt* requestPackage = NULL;
unsigned int request_size = 0;
TRipPkt* receivePackage = NULL;
unsigned int receive_size = 0;
char* pcGroupAddr = "224.0.0.9";
unsigned short group_port = 520;

void rip_entry_ntoh(TRipEntry* rip_entry){
    rip_entry->uiMetric = ntohl(rip_entry->uiMetric);
    rip_entry->usTag = ntohs(rip_entry->usTag);
    rip_entry->usFamily = ntohs(rip_entry->usFamily);
}

void rip_entry_hton(TRipEntry* rip_entry){
    rip_entry->uiMetric = htonl(rip_entry->uiMetric);
    rip_entry->usTag = htons(rip_entry->usTag);
    rip_entry->usFamily = htons(rip_entry->usFamily);
}

void requestpkt_Encapsulate()
{
	//封装请求包  command =1,version =2,family =0,metric =16
	requestPackage = (TRipPkt*)malloc(sizeof(TRipPkt));
	memset(requestPackage, 0, sizeof(TRipPkt));
	requestPackage->ucCommand = 1;
	requestPackage->ucVersion = 2;
	requestPackage->usZero = 0;
	requestPackage->RipEntries[0].usFamily = 0;
	requestPackage->RipEntries[0].uiMetric = 16;
	rip_entry_hton(&requestPackage->RipEntries[0]);
	request_size = 2*sizeof(char)+sizeof(short)+sizeof(TRipEntry);
	printf("requestpkt encapsulate finished\n");
}


/*****************************************************
*Func Name:    rippacket_Receive  
*Description:  接收rip报文
*Input:        
*	 
*Output: 
*
*Ret  ：
*
*******************************************************/
void rippacket_Receive()
{
	printf("rippacket receive\n");
	int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in local_addr;
	struct sockaddr_in remote_addr;
	socklen_t addr_len = sizeof(local_addr);

	int  iReUseddr = 1;
	if (setsockopt(socket_fd,SOL_SOCKET ,SO_REUSEADDR,(const char*)&iReUseddr,sizeof(iReUseddr))<0)
	{
		perror("setsockopt\n");
		return ;
	}
	//设置端口重用
	int  iReUsePort = 1;
	if (setsockopt(socket_fd,SOL_SOCKET ,SO_REUSEPORT,(const char*)&iReUsePort,sizeof(iReUsePort))<0)
	{
		perror("setsockopt\n");
		return ;
	}

	//本地ip设置
	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = htons(group_port);

	if(bind(socket_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) == -1){
		perror("binding the multicast!");
		return;
	}

	//把本地地址加入到组播中
	for(int i=0;i<pcLocalInterfaceNum;++i) {
		struct ip_mreq mreq;
		mreq.imr_multiaddr.s_addr = inet_addr(pcGroupAddr);
		mreq.imr_interface.s_addr = pcLocalAddr[i].s_addr;
		if (setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
			perror("setsockopt multicast!");
			return;
		}
	}
    if(receivePackage == NULL) receivePackage = (TRipPkt*)malloc(sizeof(TRipPkt));
    else memset(receivePackage, 0, sizeof(TRipPkt));
	while(1)
	{

		printf("start receive\n");
		//接收rip报文   存储接收源ip地址
		if(recvfrom(socket_fd, receivePackage, sizeof(TRipPkt), 0, (struct sockaddr*)&remote_addr, &addr_len)<0){
			printf("recvfrom err in udptalk!\n");
			return;
		}else{
			//判断command类型，request 或 response
			if(ntohl(receivePackage->RipEntries[0].uiMetric) == 16 && ntohs(receivePackage->RipEntries[0].usFamily) == 0){
				//request
				printf("receive request packet!!!!!!\n");
                request_Handle(remote_addr.sin_addr);
			}else{
				//response
				printf("receive response packet!!!!!!\n");
				response_Handle(remote_addr.sin_addr);
			}
		}
		
	}
}


/*****************************************************
*Func Name:    rippacket_Send  
*Description:  向接收源发送响应报文
*Input:        
*	  1.stSourceIp    ：接收源的ip地址，用于发送目的ip设置
*Output: 
*
*Ret  ：
*
*******************************************************/
void rippacket_Send(struct in_addr stSourceIp)
{
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in target_addr;
    memset(&target_addr, 0, sizeof(struct sockaddr_in));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(group_port);
    target_addr.sin_addr.s_addr = stSourceIp.s_addr;
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(struct sockaddr_in));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(group_port);
    for(int i=0;i<pcLocalInterfaceNum;++i){
        unsigned int local_segment = pcLocalAddr[i].s_addr & pcLocalMask[i].s_addr;
        unsigned int new_segment = stSourceIp.s_addr & pcLocalMask[i].s_addr;
        if(local_segment == new_segment){
            local_addr.sin_addr.s_addr = pcLocalAddr[i].s_addr;
            break;
        }
    }
	//设置地址重用
	int  iReUseddr = 1;
	if (setsockopt(socket_fd,SOL_SOCKET ,SO_REUSEADDR,(const char*)&iReUseddr,sizeof(iReUseddr))<0)
	{
		perror("setsockopt\n");
		return ;
	}
	//设置端口重用
	int  iReUsePort = 1;
	if (setsockopt(socket_fd,SOL_SOCKET ,SO_REUSEPORT,(const char*)&iReUsePort,sizeof(iReUsePort))<0)
	{
		perror("setsockopt\n");
		return ;
	}
    if(bind(socket_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) == -1){
        perror("binding the multicast!");
        return;
    }
	//发送
	if(sendto(socket_fd, receivePackage, receive_size, 0, (struct sockaddr*)&target_addr, sizeof(struct sockaddr_in))<0){
	    printf("sendto error!\n");
	    return;
	}
	close(socket_fd);
}

/*****************************************************
*Func Name:    rippacket_Multicast  
*Description:  组播请求报文
*Input:        
*	  1.pcLocalAddr   ：本地ip地址
*Output: 
*
*Ret  ：
*
*******************************************************/
void rippacket_Multicast(struct in_addr pcLocalAddr)
{
	//printf("rippacket multicast %s\n",inet_ntoa(pcLocalAddr));
	int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in group_addr;
	//防止绑定地址冲突
	//设置地址重用
	int  iReUseddr = 1;
	if (setsockopt(socket_fd,SOL_SOCKET ,SO_REUSEADDR,(const char*)&iReUseddr,sizeof(iReUseddr))<0)
	{
		perror("setsockopt\n");
		return ;
	}
	//设置端口重用
	int  iReUsePort = 1;
	if (setsockopt(socket_fd,SOL_SOCKET ,SO_REUSEPORT,(const char*)&iReUsePort,sizeof(iReUsePort))<0)
	{
		perror("setsockopt\n");
		return ;
	}
	
	//目的ip设置
	memset(&group_addr, 0, sizeof(struct sockaddr_in));
	group_addr.sin_family = AF_INET;
	group_addr.sin_port = htons(group_port);
	group_addr.sin_addr.s_addr = inet_addr(pcGroupAddr);

	struct sockaddr_in local_addr;
	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = pcLocalAddr.s_addr;
	local_addr.sin_port = htons(group_port);
	if(bind(socket_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) == -1){
		perror("binding the multicast!");
		return;
	}

	//0 禁止回环  1开启回环 
	int loop = 0;
	int err = setsockopt(socket_fd, IPPROTO_IP, IP_MULTICAST_LOOP,&loop, sizeof(loop));
	if(err < 0)
	{
		perror("setsockopt IP_MULTICAST_LOOP");
	}

	//set TTL
	unsigned char ttl=1;
	setsockopt(socket_fd, IPPROTO_IP,IP_MULTICAST_TTL,&ttl,sizeof(ttl));

	//set multicast interface
	if(setsockopt(socket_fd ,IPPROTO_IP,IP_MULTICAST_IF,&pcLocalAddr,sizeof(pcLocalAddr))){
		perror("setsockopt IP_MULTICAST_IF!");
		return;
	}

	//发送
	if(sendto(socket_fd, requestPackage, request_size, 0,
			(struct sockaddr*)&group_addr, sizeof(struct sockaddr_in))<0){
		printf("sendto error!\n");
		return;
	}
	//printf("sendto ok!\n");
	close(socket_fd);
}

/*****************************************************
*Func Name:    request_Handle  
*Description:  响应request报文
*Input:        
*	  1.stSourceIp   ：接收源的ip地址
*Output: 
*
*Ret  ：
*
*******************************************************/
void request_Handle(struct in_addr stSourceIp)
{
	//处理request报文
	//遵循水平分裂算法
	//回送response报文，command置为RIP_RESPONSE
	receivePackage->ucCommand = 2;
	TRtEntry* route_entry_now = g_pstRouteEntry;
	int i = 0;
	while(route_entry_now->pstNext != NULL){
		route_entry_now = route_entry_now->pstNext;
		receivePackage->RipEntries[i].stAddr.s_addr = route_entry_now->stIpPrefix.s_addr;
		receivePackage->RipEntries[i].uiMetric = route_entry_now->uiMetric;
		receivePackage->RipEntries[i].usFamily = 2;
		receivePackage->RipEntries[i].stNexthop.s_addr = route_entry_now->stNexthop.s_addr;
		receivePackage->RipEntries[i].stPrefixLen.s_addr = route_entry_now->uiPrefixLen.s_addr;
		rip_entry_hton(&receivePackage->RipEntries[i]);
		++i;
	}
	receivePackage->ucCommand = 2;
	receive_size = i*sizeof(TRipEntry)+2*sizeof(char)+sizeof(short);
	rippacket_Send(stSourceIp);
}

/*****************************************************
*Func Name:    response_Handle  
*Description:  响应response报文
*Input:        
*	  1.stSourceIp   ：接收源的ip地址
*Output: 
*
*Ret  ：
*
*******************************************************/
void response_Handle(struct in_addr stSourceIp)
{
	//处理response报文
	int i;
	for(i=0;i<RIP_MAX_ENTRY;++i){
		if(receivePackage->RipEntries[i].usFamily == 0) break;
		rip_entry_ntoh(&receivePackage->RipEntries[i]);
        if(receivePackage->RipEntries[i].uiMetric <=0 || receivePackage->RipEntries[i].uiMetric>16) continue;
		printf("Now %d ", i);
		printf("addr: %s ", inet_ntoa(receivePackage->RipEntries[i].stAddr));
		printf("nexthop: %s\n", inet_ntoa(receivePackage->RipEntries[i].stNexthop));
		receivePackage->RipEntries[i].uiMetric += 1;
        if(receivePackage->RipEntries[i].stNexthop.s_addr == stSourceIp.s_addr){
        	printf("Drop out %d\n", i);
            continue;
        } else if(receivePackage->RipEntries[i].uiMetric > 16){
			TRtEntry* route_entry_now = g_pstRouteEntry;
			TRtEntry* route_entry_before = NULL;
			while(route_entry_now->pstNext != NULL){
				route_entry_before = route_entry_now;
				route_entry_now = route_entry_now->pstNext;
				if(route_entry_now->stIpPrefix.s_addr == receivePackage->RipEntries[i].stAddr.s_addr) {
                    if(route_entry_now->stNexthop.s_addr != stSourceIp.s_addr) break;
				    route_SendForward(25, route_entry_now);
					route_entry_before->pstNext = route_entry_now->pstNext;
                    printf("Remove %d, addr: %s, metric: %d\n", i,
                           inet_ntoa(receivePackage->RipEntries[i].stAddr), receivePackage->RipEntries[i].uiMetric);
					break;
				}
			}
		} else{
			TRtEntry* route_entry_now = g_pstRouteEntry;
			int exist = 0;
			while(route_entry_now->pstNext != NULL){
				route_entry_now = route_entry_now->pstNext;
				if(route_entry_now->stIpPrefix.s_addr == receivePackage->RipEntries[i].stAddr.s_addr){
					exist = 1;
					//printf("Exist %d\n",i);
					if(receivePackage->RipEntries[i].uiMetric < route_entry_now->uiMetric) {
						route_entry_now->uiMetric = receivePackage->RipEntries[i].uiMetric;
						route_entry_now->stNexthop.s_addr = stSourceIp.s_addr;
                        for(int j=0;j<pcLocalInterfaceNum;++j){
                            if((stSourceIp.s_addr & pcLocalMask[j].s_addr) ==
                                    (pcLocalAddr[j].s_addr & pcLocalMask[j].s_addr)){
                                route_entry_now->pcIfname = pcLocalName[j];
                                break;
                            }
                        }
                        route_SendForward(24, route_entry_now);
					}
					break;
				}
			}
			if(exist == 0){
			    TRtEntry* new_entry = (TRtEntry*)malloc(sizeof(TRtEntry));
                for(int j=0;j<pcLocalInterfaceNum;++j){
                    if((stSourceIp.s_addr & pcLocalMask[j].s_addr) ==
                            (pcLocalAddr[j].s_addr & pcLocalMask[j].s_addr)){
                        new_entry->pcIfname = pcLocalName[j];
                        break;
                    }
                }
			    new_entry->stIpPrefix.s_addr = receivePackage->RipEntries[i].stAddr.s_addr;
			    new_entry->stNexthop.s_addr = stSourceIp.s_addr;
			    new_entry->uiMetric = receivePackage->RipEntries[i].uiMetric;
			    new_entry->uiPrefixLen.s_addr = receivePackage->RipEntries[i].stPrefixLen.s_addr;
			    new_entry->pstNext = g_pstRouteEntry->pstNext;
			    g_pstRouteEntry->pstNext = new_entry;
                route_SendForward(24, new_entry);
			}
		}
	}
	TRtEntry* route_entry = g_pstRouteEntry;
	if(route_entry->pstNext == NULL){
	    printf("route_entry is NULL\n");
	}
	while(route_entry->pstNext != NULL){
		route_entry = route_entry->pstNext;
        printf("metric: %d ", route_entry->uiMetric);
		printf("prefix: %s ", inet_ntoa(route_entry->stIpPrefix));
		printf("nexthop: %s\n", inet_ntoa(route_entry->stNexthop));
	}
}

/*****************************************************
*Func Name:    route_SendForward  
*Description:  响应response报文
*Input:        
*	  1.uiCmd        ：插入命令
*	  2.pstRtEntry   ：路由信息
*Output: 
*
*Ret  ：
*
*******************************************************/
void route_SendForward(unsigned int uiCmd,TRtEntry *pstRtEntry)
{
	//建立tcp短连接，发送插入、删除路由表项信息到转发引擎
	int sendfd;
	int sendlen=0;
	int tcpcount=0;
	struct sockaddr_in dst_addr;
	char buf[sizeof(struct selfroute)];

	memset(buf, 0, sizeof(buf));
	struct selfroute *selfrt;
	selfrt = (struct selfroute *)&buf;
	selfrt->selfprefixlen = 24;
	selfrt->selfprefix.s_addr	= pstRtEntry->stIpPrefix.s_addr;
	printf("before ip prefix %s\n", inet_ntoa(pstRtEntry->stIpPrefix));
	printf("ip prefix %s\n", inet_ntoa(selfrt->selfprefix));
    if(pstRtEntry->pcIfname == NULL) printf("NULL\n");
    printf("%s\n", pstRtEntry->pcIfname);
	selfrt->selfifindex	= if_nametoindex(pstRtEntry->pcIfname);
	selfrt->selfnexthop.s_addr	= pstRtEntry->stNexthop.s_addr;
	selfrt->cmdnum        = uiCmd;

	memset(&dst_addr, 0, sizeof(struct sockaddr_in));
	dst_addr.sin_family = AF_INET;
	dst_addr.sin_port   = htons(800);
	dst_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if ((sendfd = socket(AF_INET, SOCK_STREAM,0 )) == -1)
	{
		printf("self route sendfd socket error!!\n");
		exit(-1);
	}

	while(tcpcount <6)
	{
		if(connect(sendfd,(struct sockaddr*)&dst_addr,sizeof(dst_addr))<0)
		{
			tcpcount++;
		}
		else {
			break;
		}
//		sleep(1);
	}
	if(tcpcount<6)
	{
		sendlen = send(sendfd, buf, sizeof(buf), 0); //struct sockaddr *)&dst_addr, sizeof(struct sockaddr_in));
		if (sendlen <= 0)
		{
			printf("self route sendto() error!!!\n");
			exit(-1);
		}
		if(sendlen >0)
		{
			printf("send ok!!!");
		}
//		sleep(5);
		close(sendfd);
	}
}

void rippacket_Update()
{
	//printf("rippacket update\n");
    requestPackage->ucCommand = 2;
	for(int j=0;j<pcLocalInterfaceNum;++j){
        struct in_addr local_addr;
        local_addr.s_addr = pcLocalAddr[j].s_addr;
        //遍历rip路由表，封装更新报文
        TRtEntry* route_entry_now = g_pstRouteEntry;
        int i = 0;
        while(route_entry_now->pstNext != NULL){
            route_entry_now = route_entry_now->pstNext;
            if((local_addr.s_addr & route_entry_now->uiPrefixLen.s_addr) ==
                    (route_entry_now->stIpPrefix.s_addr & route_entry_now->uiPrefixLen.s_addr)) {
                continue;
            }
            else if((local_addr.s_addr & route_entry_now->uiPrefixLen.s_addr) ==
                    (route_entry_now->stNexthop.s_addr & route_entry_now->uiPrefixLen.s_addr)){
                continue;
            }
            requestPackage->RipEntries[i].usFamily = 2;
            requestPackage->RipEntries[i].uiMetric = route_entry_now->uiMetric;
            requestPackage->RipEntries[i].stNexthop.s_addr = route_entry_now->stNexthop.s_addr;
            requestPackage->RipEntries[i].stPrefixLen.s_addr = route_entry_now->uiPrefixLen.s_addr;
            requestPackage->RipEntries[i].stAddr.s_addr = route_entry_now->stIpPrefix.s_addr;
            rip_entry_hton(&requestPackage->RipEntries[i]);
            i+=1;
        }
        request_size = 2*sizeof(char)+sizeof(short)+i*sizeof(TRipEntry);
        rippacket_Multicast(local_addr);
	}
	//注意水平分裂算法
}

void* thread(void *arg){
	while(1){
		sleep(5);
		rippacket_Update();
	}
}

void ripdaemon_Start()
{
	//创建更新线程，30s更新一次,向组播地址更新Update包
	int ret = 0;
	pthread_t tid;
	ret = pthread_create(&tid, NULL, thread, NULL);
	if(ret!=0){
		perror("pthread_create fail");
		exit(0);
	}
	//封装请求报文，并组播
    requestpkt_Encapsulate();
	for(int i=0;i<pcLocalInterfaceNum;++i){
		rippacket_Multicast(pcLocalAddr[i]);
	}
	//接收rip报文
	rippacket_Receive();
}

void routentry_Insert()
{
	//将本地接口表添加到rip路由表里
	for(int i=0;i<pcLocalInterfaceNum;++i){
		g_pstRouteEntry->stIpPrefix.s_addr = (pcLocalAddr[i].s_addr & pcLocalMask[i].s_addr);
		g_pstRouteEntry->uiMetric = 1;
		g_pstRouteEntry->pcIfname = pcLocalName[i];
		g_pstRouteEntry->uiPrefixLen.s_addr = pcLocalMask[i].s_addr;
		g_pstRouteEntry->stNexthop.s_addr = inet_addr("0.0.0.0");
		printf("Add new local: name: %s, addr: %s, ",pcLocalName[i],inet_ntoa(pcLocalAddr[i]));
		printf("mask: %s\n",inet_ntoa(pcLocalMask[i]));
        TRtEntry pstRtEntry;
        pstRtEntry.stIpPrefix.s_addr = g_pstRouteEntry->stIpPrefix.s_addr;
        pstRtEntry.pcIfname = pcLocalName[i];
        pstRtEntry.stNexthop.s_addr = g_pstRouteEntry->stNexthop.s_addr;
        pstRtEntry.uiMetric = 1;
        pstRtEntry.uiPrefixLen.s_addr = g_pstRouteEntry->uiPrefixLen.s_addr;
        pstRtEntry.pstNext = NULL;
        route_SendForward(24, &pstRtEntry);
        TRtEntry* thisEntry = g_pstRouteEntry;
        g_pstRouteEntry = (TRtEntry*)malloc(sizeof(TRtEntry));
        g_pstRouteEntry->pstNext = thisEntry;
	}
}

void localinterf_GetInfo()
{
	struct ifaddrs *pstIpAddrStruct = NULL;
	struct ifaddrs *pstIpAddrStCur  = NULL;
	void *pAddrPtr=NULL;
	const char *pcLo = "127.0.0.1";
	
	getifaddrs(&pstIpAddrStruct); //linux系统函数
	pstIpAddrStCur = pstIpAddrStruct;
	
	int i = 0;
	while(pstIpAddrStruct != NULL)
	{
		if(pstIpAddrStruct->ifa_addr->sa_family==AF_INET)
		{
			pAddrPtr = &((struct sockaddr_in *)pstIpAddrStruct->ifa_addr)->sin_addr;
			char cAddrBuf[INET_ADDRSTRLEN];
			memset(&cAddrBuf,0,INET_ADDRSTRLEN);
			inet_ntop(AF_INET, pAddrPtr, cAddrBuf, INET_ADDRSTRLEN);
			if(strcmp((const char*)&cAddrBuf,pcLo) != 0)
			{
				pcLocalAddr[i].s_addr = ((struct sockaddr_in *)pstIpAddrStruct->ifa_addr)->sin_addr.s_addr;
				pcLocalName[i] = (char *)malloc(IF_NAMESIZE);
				strcpy(pcLocalName[i],(const char*)pstIpAddrStruct->ifa_name);
				pcLocalMask[i].s_addr = ((struct sockaddr_in *)pstIpAddrStruct->ifa_netmask)->sin_addr.s_addr;
				i++;
			}	
		}
		pstIpAddrStruct = pstIpAddrStruct->ifa_next;
	}
	freeifaddrs(pstIpAddrStCur);//linux系统函数
	pcLocalInterfaceNum = i;
}

int main(int argc,char* argv[])
{
	g_pstRouteEntry = (TRtEntry *)malloc(sizeof(TRtEntry));
	if(g_pstRouteEntry == NULL)
	{
		perror("g_pstRouteEntry malloc error !\n");
		return -1;
	}
	g_pstRouteEntry->pstNext = NULL;
	localinterf_GetInfo();
	printf("Num: %d\n",pcLocalInterfaceNum);
	routentry_Insert();
	ripdaemon_Start();
	return 0;
}

