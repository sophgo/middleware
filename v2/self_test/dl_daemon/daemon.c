#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
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

static char _gnetiface[32];


void GetGateway(char *pgateway)
{
	FILE *fp;
	char buf[512];
	char cmd[128];
	char *tmp;
	int err;

	strcpy(cmd, "ip route");
	fp = popen(cmd, "r");
	if (fp == NULL) {
		perror("popen error");
		return;
	}
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		tmp = buf;
		while (*tmp && isspace(*tmp))
			++tmp;
		if (strncmp(tmp, "default", strlen("default")) == 0)
			break;
	}
	err = sscanf(buf, "%*s%*s%s", pgateway);
	pclose(fp);
}

void GetNetInfo(char *paddress, char *pnetmask, char *pgateway, char *pmacaddr, const char *szDevName)
{
	int s = socket(AF_INET, SOCK_DGRAM, 0);

	if (s < 0) {
		fprintf(stderr, "Create socket failed!errno=%d", errno);
		return;
	}

	struct ifreq ifr;
	unsigned char mac[6];
	unsigned long nIP, nNetmask, nBroadIP;

	//printf("%s:\n", szDevName);

	strcpy(ifr.ifr_name, szDevName);
	if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0) {
		return;
	}
	memcpy(mac, ifr.ifr_hwaddr.sa_data, sizeof(mac));
	sprintf(pmacaddr, "%02x-%02x-%02x-%02x-%02x-%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	//("\tMAC: %02x-%02x-%02x-%02x-%02x-%02x\n",
	//		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	strcpy(ifr.ifr_name, szDevName);
	if (ioctl(s, SIOCGIFADDR, &ifr) < 0) {
		nIP = 0;
	} else {
		nIP = *(unsigned long *)&ifr.ifr_broadaddr.sa_data[2];
	}
	sprintf(paddress, "%s", inet_ntoa(*(struct in_addr *)&nIP));
	//printf("\tIP: %s\n", inet_ntoa(*(struct in_addr *)&nIP));

	strcpy(ifr.ifr_name, szDevName);
	if (ioctl(s, SIOCGIFBRDADDR, &ifr) < 0) {
		nBroadIP = 0;
	} else {
		nBroadIP = *(unsigned long *)&ifr.ifr_broadaddr.sa_data[2];
	}
	//printf("\tBroadIP: %s\n", inet_ntoa(*(struct in_addr *)&nBroadIP));

	strcpy(ifr.ifr_name, szDevName);
	if (ioctl(s, SIOCGIFNETMASK, &ifr) < 0) {
		nNetmask = 0;
	} else {
		nNetmask = *(unsigned long *)&ifr.ifr_netmask.sa_data[2];
	}
	//printf("\tNetmask: %s\n", inet_ntoa(*(struct in_addr *)&nNetmask));
	sprintf(pnetmask, "%s", inet_ntoa(*(struct in_addr *)&nNetmask));
	close(s);

	GetGateway(pgateway);
}

unsigned long get_ip_uint(const char *szDevName)
{
	int s;
	struct ifreq ifr;
	unsigned long nIP = 0;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		fprintf(stderr, "Create socket failed!errno=%d", errno);
		return nIP;
	}

	strcpy(ifr.ifr_name, szDevName);
	if (ioctl(s, SIOCGIFADDR, &ifr) < 0) {
		nIP = 0;
	} else {
		nIP = *(unsigned long *)&ifr.ifr_broadaddr.sa_data[2];
	}

	return nIP;
}

void run_reboot(void)
{
	unsigned long ip;
	char cmd_set_sram_uboot[128];
	char cmd_set_sram_ip[128];

	ip = get_ip_uint(_gnetiface);

	sprintf(cmd_set_sram_uboot, "devmem 0x03005D00 32 0x4D474E35");
	sprintf(cmd_set_sram_ip, "devmem 0x03005D04 32 0x%08x", ip);

	system(cmd_set_sram_uboot);
	system(cmd_set_sram_ip);
	system("sync");
	system("reboot -f");
}

void handle_udp_msg(int fd)
{
	char buf[BUFF_LEN];
	socklen_t len_addr;
	int count;
	struct sockaddr_in clent_addr;
	stRetMsg retmsg;
	unsigned int len_msg;

	len_addr = sizeof(clent_addr);
	len_msg = sizeof(retmsg);

	while (1) {
		memset(buf, 0, BUFF_LEN);
		count = recvfrom(fd, buf, BUFF_LEN, 0, (struct sockaddr *)&clent_addr, &len_addr);
		if (count == -1) {
			printf("receive data fail!\n");
			return;
		}

		if (strncmp(buf, CMD_HELLO, strlen(CMD_HELLO)) == 0) {
			memset(&retmsg, 0, len_msg);
			retmsg.acktype = CVI_ACK_HELLO;
			GetNetInfo((char *)&retmsg.address, (char *)&retmsg.netmask, (char *)&retmsg.gateway,
(char *)&retmsg.macaddr, _gnetiface);
			sendto(fd, &retmsg, len_msg, 0, (struct sockaddr *)&clent_addr, len_addr);
		} else if (strncmp(buf, CMD_REBOOT, strlen(CMD_REBOOT)) == 0) {
			memset(&retmsg, 0, len_msg);
			retmsg.acktype = CVI_ACK_REBOOT;
			sendto(fd, &retmsg, len_msg, 0, (struct sockaddr *)&clent_addr, len_addr);
			run_reboot();
		}
	}
}

int main(int argc, char *argv[])
{
	int server_fd, ret;
	struct sockaddr_in server_addr;

	if (argc < 3 || 0 == atoi(argv[2])) {
		printf("usage: %s network_interface port_num\n", argv[0]);
		printf("ex: %s eth0 8888\n", argv[0]);
		return -1;
	}

	strcpy((void *)&_gnetiface, argv[1]);

	const int server_port = atoi(argv[2]);

	server_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (server_fd < 0) {
		printf("create socket fail!\n");
		return -1;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(server_port);

	ret = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (ret < 0) {
		printf("socket bind fail!\n");
		return -1;
	}

	handle_udp_msg(server_fd);

	close(server_fd);

	return 0;
}
