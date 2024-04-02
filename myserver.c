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
User *create_user(char *username): create a new user with the given username
as an argument and return the address pointer to the new user struct
*/

User *create_user(char *username, int client_fd) {
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

    // set username
    ptr->username = strdup(username);
    if (ptr->username == NULL) {
        fprintf(stderr, "Out of memory in the create_user function during username setup\n");
        free(ptr);
        return NULL;
    }

	ptr->info = NULL;
	ptr->draw_match = 0;
	ptr->win_match = 0;
	ptr->loss_match = 0;
	ptr->status = 0;
	ptr->client_fd = client_fd;
	ptr->message_num = 0; 
    ptr->next = NULL;
    *ptr_ptr = ptr; /* This can help handle NULL(empty linked list) case */
    return ptr;
}

/*
User *find_user_with_name(char *username): find the existing user using the given name as argument and return 
the pointer to the user object. return NULL if not found
*/
User *find_user_with_name(char *username) {
	// pointer to head
    User *ptr = user_head;
	while (ptr != NULL) {
		if (strcmp(ptr->username, username) == 0)
			return ptr;
		ptr = ptr->next;
	}
	return NULL;
}

/*
User *find_user_with_fd(int client_fd): find the existing user using the client file descriptor as argument and return 
the pointer to the user object. return NULL if not found
*/
User *find_user_with_fd(int client_fd) {
	// pointer to head
    User *ptr = user_head;
	while (ptr != NULL) {
		if (ptr->client_fd == client_fd)
			return ptr;
		ptr = ptr->next;
	}
	return NULL;
}

// writes a given string to the given client file descriptor
void write_message(int client_fd, char *message) {
    write(client_fd, message, strlen(message) + 1);
}

// this writes the default "<USERNAME: MESSAGE_NUMBER> " pre-prended to every logged-in user prompt
void write_user_message_format(User *user, int client_fd) {
	// Estimate the size needed for the formatted string
    // username can only be 100 (size of buffer)
    // +4 for ": " and the angle brackets, +7 for the integer (1 million messages for a given user),
    // and +1 for the null terminator.
    int bufferSize = 100 + 4 + 6 + 1;
    char* message = (char*)malloc(bufferSize);

    if (message == NULL) {
        printf("Failed to allocate memory.\n");
        return;
    }

    // sprintf to write the formatted message to the allocated buffer
    sprintf(message, "<%s: %d> ", user->username, user->message_num);

    write_message(client_fd, message);

    // free the allocated memory
    free(message);

	// increment the message number
	user->message_num = user->message_num + 1;
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
					// command has been written, will need to evaluate it
					char *extractedBuf = extractString(buf);
					User *found_user = find_user_with_fd(client[i]);
					// User not found, need to create a new user or log into an existing account
					if (found_user == NULL) {
						// if the user logs in as a guest
						if (strcmp(extractedBuf, "guest") == 0) {
							User *created_guest = create_user((char *)"guest", client[i]);
							// want a new line before displaying the help menu
							write_message(client[i], (char *)"\n");
							// display the help command
							write_message(client[i], help_command());
							// display the guest user introductory message
							write_message(client[i], guest_user_message());
							write_user_message_format(created_guest, client[i]);
						}
						// the user enters a username
						else {
							User *found_user_by_name = find_user_with_name(extractedBuf);
							// if no user is found, we need to create one
							if (found_user_by_name == NULL) {
								create_user(extractedBuf, client[i]);
							}
							// if a user is found, the account already exists 
							// - we need to change the client_fd and status variables so that we can follow the logged-in user workflow
							else {
								// TODO: implement
								// status = 2
								// client_fd = client[i]
							}
						}
					}
					// if the user is logged in
					else {
						// if the logged in user is a guest, we now should only listen to the register, quit, and exit commands
						if (strcmp(found_user->username, "guest") == 0) {
							// if the first word is "register"
							if (strcmp(extractedBuf, "register") == 0) {
								// TODO: implement the register command
								// TODO: Handle case when the username registered already exists
								write_message(client[i], (char *)"register");
							}
							else if (strcmp(extractedBuf, "exit") == 0 || strcmp(extractedBuf, "quit") == 0) {
								// TODO: From the client-side this seems correct (the same behavior as the example server)
								// but will need to track which user logged out
								write_message(client[i], connection_closed_message());
								close(client[i]);
								FD_CLR(client[i], &allset);
								client[i] = -1;
							}
							// the user needs to either enter the register command or exit
							else {
								write_message(client[i], guest_user_warning_message());
							}
						}
						// the logged in user is not a guest,
						// we need to verify they have a valid password
						// we need to listen to all commands
						else {
							if (strcmp(extractedBuf, "help") == 0 || strcmp(extractedBuf, "?") == 0) {
								write_message(client[i], help_command());
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
	}
}

// TODO
// test char limit for example server read
// test control character clearing
// confirm exit and quit are aliases