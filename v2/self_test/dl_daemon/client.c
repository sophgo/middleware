#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define BUFF_LEN 1024

#define CMD_HELLO   "CVI_HELLO"
#define CMD_REBOOT  "CVI_REBOOT"

typedef enum {
	CVI_ACK_HELLO,
	CVI_ACK_REBOOT,
} eAckType;

typedef struct {
	eAckType acktype;
	char   address[32];
	char   netmask[32];
	char   gateway[32];
	char   macaddr[32];
} stRetMsg, *pstRetMsg;


void udp_msg_sender(int fd, struct sockaddr *dst)
{
	socklen_t len;
	struct sockaddr_in src;
	char buf[BUFF_LEN];
	ssize_t recv;
	stRetMsg retmsg;

	len = sizeof(*dst);

	while (1) {
		memset(buf, 0, sizeof(buf));
		strcpy(buf, CMD_HELLO);

		sendto(fd, buf, sizeof(buf), 0, dst, len);

		while (1) {
			memset(&retmsg, 0, sizeof(retmsg));
			recv = recvfrom(fd, &retmsg, sizeof(retmsg), 0, (struct sockaddr *)&src, &len);

			if (recv < 0) {
				break;
			}

			if (retmsg.acktype == CVI_ACK_HELLO) {
				char str[INET_ADDRSTRLEN];

				inet_ntop(AF_INET, &(src.sin_addr), str, INET_ADDRSTRLEN);
				printf("received from %s:%d\n", str, ntohs(src.sin_port));
				printf("==========\n");
				printf("%s\n", retmsg.address);
				printf("%s\n", retmsg.netmask);
				printf("%s\n", retmsg.gateway);
				printf("%s\n", retmsg.macaddr);
				printf("==========\n");
				printf("\n");

				//src.sin_addr.s_addr = inet_addr("192.168.0.123");
				// memset(buf, 0, sizeof(buf));
				// strcpy(buf, CMD_REBOOT);
				// sendto(fd, buf, sizeof(buf), 0, (struct sockaddr*)&src, len);
			} else if (retmsg.acktype == CVI_ACK_REBOOT) {
				char str[INET_ADDRSTRLEN];

				inet_ntop(AF_INET, &(src.sin_addr), str, INET_ADDRSTRLEN);
				printf("received from %s:%d\n", str, ntohs(src.sin_port));
				printf("==========\n");
				printf("system reboot\n");
				printf("==========\n");
				printf("\n");
			}
		}

		usleep(1000  * 10);
	}
}

int main(int argc, char *argv[])
{
	int client_fd;
	struct sockaddr_in server_addr;
	int so_broadcast;
	struct timeval tv;

	if (argc < 2 || 0 == atoi(argv[1])) {
		printf("usage: %s port_num\n", argv[0]);
		printf("ex: %s 8888\n", argv[0]);
		return -1;
	}

	const int server_port = atoi(argv[1]);

	so_broadcast = 1;

	tv.tv_sec = 1;
	tv.tv_usec =  0;

	client_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (client_fd < 0) {
		printf("create socket fail!\n");
		return -1;
	}

	if (setsockopt(client_fd, SOL_SOCKET, SO_BROADCAST, &so_broadcast, sizeof(so_broadcast)) < 0) {
		printf("setsockopt fail!\n");
		return -1;
	}

	if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		printf("setsockopt fail!\n");
		return -1;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	server_addr.sin_port = htons(server_port);

	udp_msg_sender(client_fd, (struct sockaddr *)&server_addr);

	close(client_fd);

	return 0;
}
