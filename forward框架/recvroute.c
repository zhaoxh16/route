#include "recvroute.h"

int static_route_get(struct selfroute *selfrt, int sock_fd)
{
	
    int conn_fd = accept(sock_fd, (struct sockaddr *)NULL, NULL);
	printf("Accept\n");
	fflush(stdout);
    int ret = recv(conn_fd, selfrt, sizeof(struct selfroute), 0);
    printf("%d %d\n", conn_fd, ret);
    close(conn_fd);
    return 1;
}
