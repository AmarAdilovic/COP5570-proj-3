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
#include <ctype.h>

// Initialize nulls
Game *game_head = NULL;
User *user_head = NULL;
TempUser *temp_user_head = NULL;
TempMail *temp_mail_head;
Request *request_head;

char** extractStrings(const char* input, int* count) {
    // Count the number of words to determine the size of the array of strings
    int words = 1;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == ' ' || input[i] == '\t') {
			words++;
        }
    }

    // Allocate memory for the array of strings
    char** result = (char**)malloc(words * sizeof(char*));
    if (result == NULL) {
        printf("Memory allocation for array of strings failed\n");
        return NULL;
    }

    *count = words; // Set the number of words
    int wordIndex = 0; // Index for the current word
    const char* wordStart = input; // Start of the current word

    for (int i = 0; ; i++) {
        // If space, null terminator, or tab is encountered, copy the current word
        if (input[i] == ' ' || input[i] == '\t' || input[i] == '\0') {
            int wordLength = input + i - wordStart;
			// +1 for the null terminator
            result[wordIndex] = (char*)malloc(wordLength + 1);
            if (result[wordIndex] == NULL) {
                printf("Memory allocation for string failed\n");
                // Free previously allocated strings
                while (--wordIndex >= 0) {
                    free(result[wordIndex]);
                }
                free(result);
                return NULL;
            }
            strncpy(result[wordIndex], wordStart, wordLength);
            result[wordIndex][wordLength] = '\0';
            wordIndex++;

            if (input[i] == '\0') {
                break; // Exit if end of the input string
            }
            wordStart = input + i + 1; // Move to the start of the next word
        }
    }

    return result;
}

// the buf will usually have a carriage return and new line appended to the end,
// however, sometimes it may have an undefined number of new lines, carriage returns, or other characters
// for example, when entering "?" on the client, the server received "?\r\np\r\n\n" or "?\r\nt\n\r\n\x01"
char* extractString(const char* input) {
	// Determine the length of the printable character sequence (assuming non-printable characters do not occur in the middle of the string)
	int length = 0;
	for (int i = 0; input[i] != '\0'; ++i) {
		unsigned char ch = input[i]; // Use unsigned char to handle characters correctly
		if ((ch >= 32 && ch <= 126) || ch == 9) {
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

char* combineUserInputs(char** userInputs, int numWords) {
    if (numWords == 0 || numWords == 1) {
        return "";
    }

    // Calculate the total length needed for the combined string
    size_t totalLength = 0;
    for (int i = 1; i < numWords; i++) {
		// +1 = add null terminator
        totalLength += strlen(userInputs[i]) + 1;
    }

    char* combined = malloc(totalLength);
    if (combined == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    strcpy(combined, userInputs[1]);

    // Append the rest of the strings, delimited by spaces
    for (int i = 2; i < numWords; i++) {
        strcat(combined, " ");
        strcat(combined, userInputs[i]);
    }

    return combined;
}


char* trimSpaces(const char* input) {
    if (input == NULL) {
        return NULL;
    }

    // Pointers to the start and end of the input string
    const char* start = input;
    const char* end = input + strlen(input) - 1;

    // Advance the starting pointer to the first non-space character
    while (isspace((unsigned char)*start) && start <= end) {
        start++;
    }

    // Move the end pointer back to the last non-space character
    while (isspace((unsigned char)*end) && end >= start) {
        end--;
    }

    // Calculate the new string length
    size_t length = end - start + 1;

    // Allocate memory for the trimmed string (+1 for the null terminator)
    char* trimmed = (char*)malloc(length + 1);
    if (trimmed == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    strncpy(trimmed, start, length);
    trimmed[length] = '\0';

    return trimmed;
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

	// we initialize info to an empty string for easier evaluation later
	ptr->info = "";
	ptr->draw_match = 0;
	ptr->win_match = 0;
	ptr->loss_match = 0;
	ptr->status = USER_OFFLINE_STATUS;
	ptr->quiet = 0;
	ptr->client_fd = client_fd;
	ptr->message_num = 0; 
    ptr->next = NULL;
    *ptr_ptr = ptr; /* This can help handle NULL(empty linked list) case */
    return ptr;
}

/*
TempUser *create_temp_user(char *username): create a new user with the given username
as an argument and return the address pointer to the new temp user struct
*/

TempUser *create_temp_user(char *username, int client_fd) {
    // pointer to pointer of head
    TempUser **ptr_ptr = &temp_user_head;
    // pointer to head
    TempUser *ptr = temp_user_head;

    // find a place to add new temp user
    while (ptr != NULL) {
        ptr_ptr = &(ptr->next);
        ptr = ptr->next;
    }

    // allocate memory for new temp user
    ptr = malloc(sizeof(TempUser));
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory when using create_temp_user function\n");
        return NULL;
    }

    // set username
    ptr->username = strdup(username);
    if (ptr->username == NULL) {
        fprintf(stderr, "Out of memory in the create_temp_user function during username setup\n");
        free(ptr);
        return NULL;
    }

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

/*
TempUser *find_temp_user_with_name(char *username): find the existing TempUser using the given name as argument and return 
the pointer to the TempUser object. return NULL if not found
*/
TempUser *find_temp_user_with_name(char *username) {
	// pointer to head
    TempUser *ptr = temp_user_head;
	while (ptr != NULL) {
		if (strcmp(ptr->username, username) == 0)
			return ptr;
		ptr = ptr->next;
	}
	return NULL;
}

/*
TempUser *find_temp_user_with_fd(int client_fd): find the existing TempUser using the client file descriptor as argument and return 
the pointer to the TempUser object. return NULL if not found
*/
TempUser *find_temp_user_with_fd(int client_fd) {
	// pointer to head
    TempUser *ptr = temp_user_head;
	while (ptr != NULL) {
		if (ptr->client_fd == client_fd)
			return ptr;
		ptr = ptr->next;
	}
	return NULL;
}

/*
TempUser *free_temp_user(TempUser *temp_user_pointer): free the TempUser
*/
void free_temp_user(TempUser *temp_user_pointer) {
	printf("Clearing the TempUser: %s\n", temp_user_pointer->username);
    // pointer to head
	TempUser *cur = temp_user_head;

    // if the TempUser we want to delete is at the beginning of the list
    if (cur == temp_user_pointer) {
        temp_user_head = temp_user_pointer->next;
    } else {
        while (cur->next != temp_user_pointer) {
            cur = cur->next;
            if (cur == NULL) {
                fprintf(stderr, "TempUser does not exist -- error in free_temp_user\n");
                return;
            }
        }
        cur->next = temp_user_pointer->next;
    }

    // free allocated memory in temp_user_pointer
    free(temp_user_pointer->username);
    free(temp_user_pointer->password);
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
    int bufferSize = 100 + 4 + 7 + 1;
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

// this writes the default "<USERNAME: MESSAGE_NUMBER> " pre-prended to every user prompt
void write_temp_user_message_format(TempUser *user, int client_fd) {
	// Estimate the size needed for the formatted string
    // username can only be 100 (size of buffer)
    // +4 for ": " and the angle brackets, +7 for the integer (1 million messages for a given user),
    // and +1 for the null terminator.
    int bufferSize = 100 + 4 + 7 + 1;
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

void log_user_in(User *found_user_by_name, int client_fd) {
	printf("User %s loging in, with client = %d and status = %d\n", found_user_by_name->username, client_fd, found_user_by_name->status);

	// we set the user to online and change their file descriptor
	found_user_by_name->status = USER_ONLINE_STATUS;
	found_user_by_name->client_fd = client_fd;

	// display the welcome banner
	write_message(client_fd, welcome_message());
	// display the help command
	write_message(client_fd, help_command());
	// TODO: Check for unread messages here
	// write_message(client[i], guest_user_message());
	write_user_message_format(found_user_by_name, client_fd);
}

int close_client_connection(int client_fd, fd_set *allset) {
	printf("Closing client %d connection\n", client_fd);
	write_message(client_fd, connection_closed_message());
	close(client_fd);
	FD_CLR(client_fd, allset);
	return -1;
}

int close_user_connection(User *exitting_user, int client_fd, fd_set *allset) {
	exitting_user->status = USER_OFFLINE_STATUS;
	// we want to reset the message number
	exitting_user->message_num = 0;
	return close_client_connection(client_fd, allset);
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
	int maxfd, time, check_flag;
	User *opponent;
	char piece;
	char *temp, *user_temp_str; // temp is an allocate space DO NOT FREE, user_temp_str is pointer
	Game *game_ptr;

	temp = (char*) malloc(100*sizeof(char));

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

			printf("remote machine = %s, port = %d.\n",
					inet_ntoa(recaddr.sin_addr), ntohs(recaddr.sin_port)); 

			for (i=0; i<MAXCONN; i++) {
				if (client[i] < 0) {
					client[i] = rec_sock; 
					FD_SET(client[i], &allset);

					write_message(client[i], initial_messsage());

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
				printf("reading from = %d with buf=%s\n", client[i], buf);
				if (num == 0) {
					/* client exits */
					client[i] = close_client_connection(client[i], &allset);;
				} else {
					// command has been written, will need to evaluate it
					char *extractedBuf = extractString(buf);
					char *trimmedString = trimSpaces(extractedBuf);
					int numWords = 0;
    				char** userInputs = extractStrings(trimmedString, &numWords);
					char *userInput = userInputs[0];
					printf("extractedBuf = %s trimmedString = %s numWords = %d\n", extractedBuf, trimmedString, numWords);

					User *found_user = find_user_with_fd(client[i]);
					TempUser *found_temp_user = find_temp_user_with_fd(client[i]);
					printf("this is the client_fd being handeled = %d\n", client[i]);
					// Initial user workflow
					// User and TempUser not found, need to create a guest or log into an existing account
					// We also do this if we do find a user, but that user is offline in our system
					if (
						(found_user == NULL && found_temp_user == NULL) ||
						(found_temp_user == NULL && found_user != NULL && found_user->status == USER_OFFLINE_STATUS)
						) {
						printf("user not found, this is their extracted input = %s and first input = %s\n", trimmedString, userInput);
						// if the user logs in as a guest
						if (strcmp(trimmedString, "guest") == 0) {
							printf("handling guest \n");
							TempUser *created_guest = create_temp_user((char *)"guest", client[i]);
							created_guest->status = TEMP_USER_GUEST_STATUS;
							// want a new line before displaying the help menu
							write_message(client[i], (char *)"\n");
							// display the help command
							write_message(client[i], help_command());
							// display the guest user introductory message
							write_message(client[i], guest_user_message());
							write_temp_user_message_format(created_guest, client[i]);
						}
						// the user enters a username
						else {
							printf("handling username \n");
							User *found_user_by_name = find_user_with_name(trimmedString);
							// if no user is found, we need to prompt them for a password
							// we create a temporary user where the username is an empty string, 
							// we do this so we can associate a client file descriptor with a temporary user object
							// we then set a flag on this user so that we know we have to get rid of it
							if (found_user_by_name == NULL) {
								printf("no user found \n");
								TempUser *created_temp = create_temp_user("", client[i]);
								// no username is found but we still need to prompt for the password
								created_temp->status = TEMP_USER_WAITING_PASSWORD_NOT_FOUND_STATUS;
								// prompt the user to enter their password for their next read iteration
								write_message(client[i], (char *)"password: ");
							}
							// if a user is found, the account already exists
							// we create a temp user for the given client file descriptor to avoid modifying the existing user account
							else {
								printf("user found \n");
								TempUser *created_temp = create_temp_user(trimmedString, client[i]);

								// there is an active connection on this account,
								// we want to remember to revert everything if the user doesn't know the correct credentials
								if (found_user_by_name->status == USER_ONLINE_STATUS) {
									created_temp->status = TEMP_USER_ACTIVE_CONNECTION_PENDING_STATUS;
								}
								// the user doesn't have an active connection, we can just do what we need
								else {
									// we found this username but we still need to validate and prompt the password
									created_temp->status = TEMP_USER_WAITING_ON_PASSWORD_STATUS;
								}
								// prompt the user to enter their password for their next read iteration
								write_message(client[i], (char *)"password: ");
							}
						}
					}
					// we have a guest or a user trying actively logging in
					else if (found_temp_user != NULL) {
						printf("TempUser found!, this is their name = %s and status = %d\n", found_temp_user->username, found_temp_user->status);
						// handle the guest
						if (found_temp_user->status == TEMP_USER_GUEST_STATUS) {
							// if the first word is "register"
							if (strcmp(userInput, "register") == 0) {
								// if the user only enters "register"
								if (numWords == 1) {
									register_command(client[i], found_temp_user, "", "");
								}
								// if the user only enters "register USERNAME"
								else if (numWords == 2) {
									register_command(client[i], found_temp_user, userInputs[1], "");
								}
								// if the user enters "register USERNAME PASSWORD", where anything after PASSWORD is ignored
								else {
									register_command(client[i], found_temp_user, userInputs[1], userInputs[2]);
								}
							}
							else if (strcmp(userInput, "exit") == 0 || strcmp(userInput, "quit") == 0) {
								client[i] = close_client_connection(client[i], &allset);
								free_temp_user(found_temp_user);
							}
							// the guest needs to either enter the register command or exit
							else {
								write_message(client[i], guest_user_warning_message());
							}
						}
						// no username has been found for this user,
						// instead of processing their password,
						// we just kill their connection and free the temp user
						else if (found_temp_user->status == TEMP_USER_WAITING_PASSWORD_NOT_FOUND_STATUS) {
							printf("WAITING_PASSWORD_NOT_FOUND_STATUS.\n");
							write_message(client[i], "Login failed!!\n");
							client[i] = close_client_connection(client[i], &allset);
							free_temp_user(found_temp_user);
						}
						// we previously found a username for this user
						else if (found_temp_user->status == TEMP_USER_WAITING_ON_PASSWORD_STATUS || found_temp_user->status == TEMP_USER_ACTIVE_CONNECTION_PENDING_STATUS) {
							printf("TEMP_USER_WAITING_ON_PASSWORD_STATUS OR ACTIVE, found_temp_user->status = %d.\n", found_temp_user->status);
							// do another lookup to retrieve the username we previously found
							User *found_user_by_name = find_user_with_name(found_temp_user->username);
							// if the entered password matches, this user can be logged in
							if (strcmp(found_user_by_name->password, userInput) == 0) {
								// we previously found an active connection for this user
								if (found_temp_user->status == TEMP_USER_ACTIVE_CONNECTION_PENDING_STATUS) {									
									write_message(found_user_by_name->client_fd, "You login from another place.\n");
									close_user_connection(found_user_by_name, found_user_by_name->client_fd, &allset);
								}
								log_user_in(found_user_by_name, client[i]);
							}
							// otherwise, the password does not match
							else {
								write_message(client[i], "Login failed!!\n");
								client[i] = close_client_connection(client[i], &allset);
							}
							free_temp_user(found_temp_user);
						}

					}
					// if the user is logged in
					// we explicitly check that there is no temp user at this file descriptor to avoid improperly clearing them
					else if (found_temp_user == NULL && found_user != NULL) {
						printf("found_user.username = %s, client = %d.\n", found_user->username, client[i]);

						// we need to listen to all commands
						if (strcmp(userInput, "help") == 0 || strcmp(userInput, "?") == 0) {
							write_message(client[i], help_command());
							write_user_message_format(found_user, client[i]);
						}
						else if (strcmp(userInput, "register") == 0) {
							write_message(client[i], "Please use a guest login to register new users.\n");
							write_user_message_format(found_user, client[i]);
						}
						else if (strcmp(userInput, "who") == 0) {
							who_command(client[i]);
							write_user_message_format(found_user, client[i]);
						}
						else if (strcmp(userInput, "game") == 0) {
							// TODO: test after the game-related functions are implemented
							write_message(client[i], print_games());
							write_user_message_format(found_user, client[i]);
						}
						else if (strcmp(userInput, "match") == 0) {
							// match command
							if (numWords != 3 && numWords != 4) {
								write_message(client[i], "match <name> <b|w> [t]\n");
								write_user_message_format(found_user, client[i]);
							} else {
								opponent = find_user_with_name(userInputs[1]); 
								if (opponent == NULL) {
									write_message(client[i], "User not exist!\n");
									write_user_message_format(found_user, client[i]);
								} else if (strcmp(opponent->username, found_user->username) == 0) {
									write_message(client[i], "You cannot have a match with yourself.\n");
									write_user_message_format(found_user, client[i]);
								} else if (opponent->status == 0) {
									write_message(client[i], "User is not online\n");
									write_user_message_format(found_user, client[i]);
								} else if (userInputs[2][0] != 'b' && userInputs[2][0] != 'w') {
									write_user_message_format(found_user, client[i]);
									continue;
								} else {
									// TODO: check if the last argument is number or not
									time = 600;
									if (numWords == 4) {
										time = atoi(userInputs[3]);
									} 
									printf("DEBUG: time %d\n ", time);
									piece = userInputs[2][0];
									// check if request exist
									check_flag = check_request(found_user->username, opponent->username, piece, time);
									printf("DEBUG: check_flag %d\n ", check_flag);
									if (check_flag == 0) {
										// new request
										create_request(found_user->username, opponent->username, piece, time);
										// let other player know 
										if (piece == 'w') {
											piece = 'b';
										} else {
											piece = 'w';
										}

										sprintf(temp, "%s invite your for a game <match %s %c %d>\n", found_user->username, found_user->username, piece, time);
										write_message(opponent->client_fd, temp);
										write_user_message_format(found_user, found_user->client_fd);
										write_user_message_format(opponent, opponent->client_fd);
									} else if (check_flag == 1) {
										// request exist and all information match

										// delete request
										delete_request_user(opponent->username);
										delete_request_user(found_user->username);

										// create game
										if (piece == 'b') {
											// create game
											game_ptr = create_game(found_user->username, opponent->username, time);
											// get board
											user_temp_str = print_board(game_ptr);
											if (user_temp_str == NULL) {
												printf("Error: not found board!");
												return 1;
											}
											// send board to both user
											write_message(opponent->client_fd, user_temp_str);
											write_message(found_user->client_fd, user_temp_str);
											write_user_message_format(opponent, opponent->client_fd);
											write_user_message_format(found_user, found_user->client_fd);
											// free the board string
											free(user_temp_str);
										}

									} else {
										// the detail of game is different 
										
										// create new request for the game
										create_request(found_user->username, opponent->username, piece, time);
										
										// print Warning string to user
										sprintf(temp,"%s; %s.\n", get_request(opponent->username, found_user->username), get_request(found_user->username, opponent->username));
										write_message(opponent->client_fd, temp);
										write_message(found_user->client_fd, temp);
										write_user_message_format(opponent, opponent->client_fd);
										write_user_message_format(found_user, found_user->client_fd);
									}
								}
							}
						}
						else if (strcmp(userInput, "passwd") == 0) {
							// if the user only enters "passwd"
							if (numWords == 1) {
								change_password_command(found_user, "");
							}
							// if the user enters "passwd NEW_PASSWORD", where anything after NEW_PASSWORD is ignored
							else {
								change_password_command(found_user, userInputs[1]);
							}
							write_message(client[i], "Password changed.\n");
							write_user_message_format(found_user, client[i]);
						}
						else if (strcmp(userInput, "stats") == 0) {
							// if the user only enters "stats"
							// treated as "stats CURR_USER_NAME"
							if (numWords == 1) {
								stats_command(client[i], found_user->username);
							}
							// if the user enters "stats USER_NAME", where anything after USER_NAME is ignored
							else {
								stats_command(client[i], userInputs[1]);
							}
							write_user_message_format(found_user, client[i]);
						}
						else if (strcmp(userInput, "info") == 0) {
							// if the user only enters "info"
							if (numWords == 1) {
								info_command(found_user, (char **)"", 0);
							}
							// if the user enters "info ANY STRING"
							else {
								info_command(found_user, userInputs, numWords);
							}
							write_message(client[i], "Info changed.\n");
							write_user_message_format(found_user, client[i]);
						}
						else if (strcmp(userInput, "exit") == 0 || strcmp(userInput, "quit") == 0) {
							client[i] = close_user_connection(found_user, client[i], &allset);
						}
						else if (strcmp(userInput, "") == 0) {
							write_user_message_format(found_user, client[i]);
						}
						else {
							write_message(client[i], "Command not supported.\n");
							write_user_message_format(found_user, client[i]);
						}
					}
					else {
						printf("NONE OF THE STATES WORKED! Debugging: \n");
						if (found_user == NULL) {
							printf("found_user is null \n");
						} else {
							printf("found_user is NOT null \n");
							printf("found_user->username: %s \n", found_user->username);
							printf("found_user->client_fd: %d \n", found_user->client_fd);
						}
						if (found_temp_user == NULL) {
							printf("found_temp_user is null \n");
						} else {
							printf("found_temp_user is NOT null \n");
							printf("found_temp_user->username: %s \n", found_temp_user->username);
							printf("found_temp_user->client_fd: %d \n", found_temp_user->client_fd);
						}
					}
					// Free the array of strings
					free(userInputs); 
				}
				// clear the buf
				memset(buf, 0, sizeof(buf));
			}
		}
	}
}
