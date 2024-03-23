// I/O Multiplexing using select()
// this is covered on lect14_cliserver.pptx slide 10

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>

int main(int argc, char * argv[])
{
	int sockfd, i, maxf;
	struct sockaddr_in addr;
	char buf[100];
	fd_set  rset, orig_set;

	if (argc < 3) {
		printf("Usage: a.out ip_addr port.\n");
		exit(0);
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror(": Can't get socket");
		exit(1);
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons((short)atoi(argv[2]));
	addr.sin_addr.s_addr = inet_addr(argv[1]);

	if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror(": connect");
		exit(1);
	}

	i = 100;
	printf("Send %x\n", i);
	write(sockfd, &i, sizeof(i));

	FD_ZERO(&orig_set);
	FD_SET(STDIN_FILENO, &orig_set);
	FD_SET(sockfd, &orig_set);    
	if (sockfd > STDIN_FILENO) maxf = sockfd+1;
	else maxf = STDIN_FILENO+ 1;

	while (1) {
		rset = orig_set;
		select(maxf, &rset, NULL, NULL, NULL);
		if (FD_ISSET(sockfd, &rset)) {
			if (read(sockfd, buf, 100) == 0) {
				printf("server crashed.\n");
				exit(0);
			}
			printf("Server response : %s\n", buf);
		}
		if (FD_ISSET(STDIN_FILENO, &rset)) {
			if (fgets(buf, 100, stdin) == NULL) exit(0);
			write(sockfd, buf, strlen(buf)+1);
		}
	}
}



