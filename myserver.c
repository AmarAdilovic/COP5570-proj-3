#define _POSIX_SOURCE
#include "myserver.h"
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>

// Initialize nulls
Game *game_head = NULL;
User *user_head = NULL;


// the buf will usually have a carriage return and new line appended to the end,
// however, sometimes it may have an undefined number of new lines, carriage returns, or other characters
// for example, when entering "?" on the client, the server received "?\r\np\r\n\n" or "?\r\nt\n\r\n\x01"
char* extractString(const char* input) {
	// Determine the length of the printable character sequence (assuming non-printable characters do not occur in the middle of the string)
	int length = 0;
	for (int i = 0; input[i] != '\0'; ++i) {
		unsigned char ch = input[i]; // Use unsigned char to handle characters correctly
		if (ch >= 32 && ch <= 126) {
			length++;
		} else {
			break;
		}
	}

    // Allocate memory for the new string (+ null terminator)
    char* result = (char*)malloc(length + 1);
    if (result == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    // Copy the printable characters to the new string
    strncpy(result, input, length);
    result[length] = '\0'; // Null-terminate the new string

    return result;
}

/*
User *create_user(char *username, char *password): create a new user with given username and password
as argument and return the address pointer to the new user struct
*/

User *create_user(char *username, char *password) {
    // pointer to pointer of head
    User **ptr_ptr = &user_head;
    // pointer to head
    User *ptr = user_head;

    // find a place to add new user
    while (ptr != NULL) {
        ptr_ptr = &(ptr->next);
        ptr = ptr->next;
    }

    // allocate memory for new user
    ptr = malloc(sizeof(User));
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory when using create_user function\n");
        return NULL;
    }

    // set username and password
    ptr->username = strdup(username);
    if (ptr->username == NULL) {
        fprintf(stderr, "Out of memory when using create_user function\n");
        free(ptr);
        return NULL;
    }

    ptr->password = strdup(password);
    if (ptr->password == NULL) {
        fprintf(stderr, "Out of memory when using create_user function\n");
        free(ptr);
        return NULL;
    }

	ptr->draw_match = 0;
	ptr->win_match = 0;
	ptr->loss_match = 0;
	ptr->status = 0;
	ptr->client_fd = -1;
	ptr->message_num = 0; 
    ptr->next = NULL;
    *ptr_ptr = ptr; /* This can help handle NULL(empty linked list) case */
    return ptr;
}

/*
User *find_user(char *username): find the existing user using the given name as argument and return 
the pointer to the user object. return NULL if not found
*/
User *find_user(char *username) {
	// pointer to head
    User *ptr = user_head;
	while (ptr != NULL) {
		if (strcmp(ptr->username, username) == 0)
			return ptr;
		ptr = ptr->next;
	}
	return NULL;

}

void sig_chld(int signo)
{
	pid_t pid;
	int stat;
	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) 
		printf("child %d terminated.\n", pid);
	return ;
}

/* 21. You can assume the maximum number of players online to have a reasonable limit (e.g. 20). */
#define MAXCONN 20

int main(int argc, char * argv[])
{
	int sockfd, rec_sock, len, i;
	struct sockaddr_in addr, recaddr;
	struct sigaction abc;
	int client[MAXCONN];
	// User *users;
	char buf[100];
	fd_set allset, rset;	
	int maxfd;

	abc.sa_handler = sig_chld;
	sigemptyset(&abc.sa_mask);
	abc.sa_flags = 0;

	sigaction(SIGCHLD, &abc, NULL);

	if (argc < 2) {
		printf("Usage: ./myserver PORT_NUMBER\n");
		exit(0);
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror(": Can't get socket");
		exit(1);
	}

	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons((short)atoi(argv[1]));

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror(": bind");
		exit(1);
	}


	len = sizeof(addr);
	if (getsockname(sockfd, (struct sockaddr *)&addr, (socklen_t *)&len) < 0) {
		perror(": can't get name");
		_exit(1);
	}

	printf("ip = %s, port = %d\n", inet_ntoa(addr.sin_addr), htons(addr.sin_port));
	printf("To connect to the server: `telnet %s %d`\n", inet_ntoa(addr.sin_addr), htons(addr.sin_port));

	if (listen(sockfd, 5) < 0) {
		perror(": bind");
		exit(1);
	}

	for (i=0; i<MAXCONN; i++) client[i] = -1;

	FD_ZERO(&allset);
	FD_SET(sockfd, &allset);
	maxfd = sockfd;

	while (1) {
		rset = allset;
		select(maxfd+1, &rset, NULL, NULL, NULL);
		if (FD_ISSET(sockfd, &rset)) {
			/* somebody tries to connect */
			if ((rec_sock = accept(sockfd, (struct sockaddr
								*)(&recaddr), (socklen_t *)&len)) < 0) {
				if (errno == EINTR)
					continue;
				else {
					perror(":accept error");
					exit(1);
				}
			}

			/*
			   if (rec_sock < 0) {
			   perror(": accept");
			   exit(1);
			   }
			   */

			/* print the remote socket information */

			printf("remote machine = %s, port = %d.\n",
					inet_ntoa(recaddr.sin_addr), ntohs(recaddr.sin_port)); 

			for (i=0; i<MAXCONN; i++) {
				if (client[i] < 0) {
					client[i] = rec_sock; 
					FD_SET(client[i], &allset);

					// add one for the null terminator
					write(client[i], initial_messsage(), strlen(initial_messsage()) + 1);

					break;
				}
			}
			if (i == MAXCONN) {
				printf("too many connections.\n");
				close(rec_sock);
			}
			if (rec_sock > maxfd) maxfd = rec_sock;
		}

		for (i=0; i<MAXCONN; i++) {
			if (client[i] < 0) continue;
			if (FD_ISSET(client[i], &rset)) {
				int num;
				num = read(client[i], buf, 100);
				if (num == 0) {
					/* client exits */
					close(client[i]);
					FD_CLR(client[i], &allset);
					client[i] = -1;
				} else {
					// command has been written, need to evaluate it now
					char *extractedBuf = extractString(buf);
					if (strcmp(extractedBuf, "help") == 0 || strcmp(extractedBuf, "?") == 0) {
						write(client[i], help_command(), strlen(help_command()) + 1);
					}
					else if (strcmp(extractedBuf, "exit") == 0 || strcmp(extractedBuf, "quit") == 0) {
						// TODO: From the client-side this seems correct (the same behavior as the example server)
						// but will need to track which user logged out
						close(client[i]);
						FD_CLR(client[i], &allset);
						client[i] = -1;
					}
					else {
						// TODO: This should be a catch-all for unknown commands, for now echo the command back to the client
						write(client[i], buf, num);
					}
				}
			}
		}
	}
}

// TODO
// test char limit for example server read
// test control character clearing
// confirm exit and quit are aliases