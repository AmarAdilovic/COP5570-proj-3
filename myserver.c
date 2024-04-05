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

void removeFirstWords(char *array[], int totalElements, int i) {
    if (i >= totalElements) {
        // If i is equal to or larger than totalElements clear the array by setting the first element to NULL.
        array[0] = NULL;
        return;
    }

    int j;
    for (j = 0; j < totalElements - i; ++j) {
        array[j] = array[j + i];
    }

    // Set the new end of the array to NULL if the array had a NULL terminator.
    array[j] = NULL;
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
	ptr->block_head = NULL;
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

int create_blocked_user(char *username, User *user) {
	// pointer to pointer of head
    BlockedUser **ptr_ptr = &user->block_head;
    // pointer to head
    BlockedUser *ptr = user->block_head;

    // find a place to add new observer
    while (ptr != NULL) {
        ptr_ptr = &(ptr->next);
        ptr = ptr->next;
    }

    // allocate memory for new observer
    ptr = malloc(sizeof(BlockedUser));
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory when using create_blocked_user function\n");
        return 1;
    }

    // set username
    ptr->username = strdup(username);
    if (ptr->username == NULL) {
        fprintf(stderr, "Out of memory in the create_blocked_user function during username setup\n");
        free(ptr);
        return 1;
    }
	
	ptr->next = NULL;
    *ptr_ptr = ptr;

    return 0;
}

void delete_blocked_user(char *username, User *user) {
    // pointer to head
    BlockedUser *ptr = user->block_head;
    BlockedUser *cur;
    cur = ptr;

    // error: BlockedUser empty
    if (user->block_head == NULL) {
        return;
    }

    // if the blocked user we want to delete is at the beginning of the list
    if (strcmp(ptr->username, username) == 0) {
        user->block_head = ptr->next;
    } else {
        while (ptr->next != NULL && strcmp(ptr->next->username, username) != 0) {
            ptr = ptr->next;
        }
        if (ptr->next == NULL) {
            fprintf(stderr, "User not exist in blocked user list -- error in delete_blocked_user\n");
            return;
        }
        cur = ptr->next;
        ptr->next = cur->next;
    }
    free(cur);
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

BlockedUser *find_blocked_user_with_name(char *username, BlockedUser *blocked_user_head) {
	// pointer to head
    BlockedUser *ptr = blocked_user_head;
	while (ptr != NULL) {
		if (strcmp(ptr->username, username) == 0)
			return ptr;
		ptr = ptr->next;
	}
	return NULL;
}

int count_online_users() {
    User *ptr = user_head;
    int numOnlineUsers = 0;
	while (ptr != NULL) {
        if (ptr->status == USER_ONLINE_STATUS) {
		    numOnlineUsers += 1;
        }
		ptr = ptr->next;
	}

    return numOnlineUsers;
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


/*
void disconnect_lost(User *user): will check if there is any game the given user is play,
if yes, resign from the game, change players' status.
*/
void disconnect_lose(User *user) {
	Game *game_ptr;
	Observer *cur;
	User *opponent;
	char *temp;
	game_ptr = find_game(user->username);

	// check if game exit
	if (game_ptr == NULL) {
		return;
	} 

	cur = game_ptr->observer_head;
	temp = (char*) malloc(100*sizeof(char)); 
	// set opponent
	if (strcmp(game_ptr->black, user->username) == 0) {
		opponent = find_user_with_name(game_ptr->white);
	} else {
		opponent = find_user_with_name(game_ptr->black);
	}

	// update player status
	opponent->win_match++;
	user->loss_match++;

	// create message 
	sprintf(temp, "%s lose due to disconnected!\n");

	// notify all the observer
	while (cur != NULL) {
		write_message(cur->user->client_fd, temp);
		write_user_message_format(cur->user, cur->user->client_fd);
		cur = cur->next;
	}

	// let the opponent know
	write_message(opponent->client_fd, temp);
	write_user_message_format(opponent, opponent->client_fd);

	// delete the game
	delete_game(game_ptr);
}

int close_user_connection(User *exitting_user, int client_fd, fd_set *allset) {
	Game *game_ptr;
	exitting_user->status = USER_OFFLINE_STATUS;
	// we want to reset the message number
	exitting_user->message_num = 0;

	// disconnect the user and make user lose the game
	disconnect_lose(exitting_user);

	// delete all user request
	delete_request_user(exitting_user->username);

	// delete user from all the observe game
	for (game_ptr = game_head; game_ptr != NULL; game_ptr = game_ptr->next) {
		if (check_observer(game_ptr, exitting_user->username) == 0) {
			delete_observer(game_ptr, exitting_user);
		} 
	}

	return close_client_connection(client_fd, allset);
}

/*
void observer_update(Game *cur_game, char *str): update new message to all observers in the game
*/
void observer_update(Game *cur_game, char *str) {
	Observer *cur = cur_game->observer_head;

	while (cur != NULL) {
		write_message(cur->user->client_fd, str);
		write_user_message_format(cur->user, cur->user->client_fd);
		cur = cur->next;
	}
}

void move_game(User *user, char* player_move) {
	printf("DEBUG: move_game %s %s", user->username, player_move);
	User *opponent;
	Game *cur_game;
	int piece = 1;
	int result = 0;
	char *temp;
	cur_game = find_game(user->username);
	
	if (cur_game == NULL) {
		// palyer not in game
		write_message(user->client_fd, "Command not supported.\n");
		write_user_message_format(user, user->client_fd);
		return;
	}

	// check what side current user in
	if (strcmp(user->username, cur_game->black) == 0) {
		piece = 2;
	}

	// set opponent
	opponent = find_user_with_name(cur_game->black);
	if (piece == 2) {
		opponent = find_user_with_name(cur_game->white);
	}

	// check if this is user turn
	if (piece != who_move(cur_game)) {
		// not player turn
		write_message(user->client_fd, "Not your turn!\n");
		write_user_message_format(user, user->client_fd);
	} else {
		result = move(player_move, cur_game);
		// if the move is invalid, we do not need to print the board
		if (result > -1) {
			temp = print_board(cur_game);

			// print board for players and observers
			write_message(user->client_fd, temp);
			write_message(opponent->client_fd, temp);
			observer_update(cur_game, temp);

			free(temp);
		}

		if (result == -1) {
            write_message(user->client_fd, "Place is already taken \n");
			write_user_message_format(user, user->client_fd);
        } else if (result == -2) {
            write_message(user->client_fd, "Input is not in right format \n");
			write_user_message_format(user, user->client_fd);
        } else if (result == 1) {
            write_message(user->client_fd, "white win \n");
			write_user_message_format(user, user->client_fd);
			write_message(opponent->client_fd, "white win \n");
			write_user_message_format(opponent, opponent->client_fd);
			observer_update(cur_game, "white win \n");
			if (piece == 1) {
				// user is white
				user->win_match++;
				opponent->loss_match++;
			} else {
				// user is black
				user->loss_match++;
				opponent->win_match++;;
			}
        } else if (result == 2) {
            write_message(user->client_fd, "Black win \n");
			write_user_message_format(user, user->client_fd);
			write_message(opponent->client_fd, "Black win \n");
			write_user_message_format(opponent, opponent->client_fd);
			if (piece == 1) {
				// user is white
				user->loss_match++;
				opponent->win_match++;
			} else {
				// user is black
				user->win_match++;
				opponent->loss_match++;
			}
			observer_update(cur_game, "Black win \n");
        } else if (result == 3) {
            write_message(user->client_fd, "Draw\n");
			write_user_message_format(user, user->client_fd);
			write_message(opponent->client_fd, "Draw \n");
			write_user_message_format(opponent, opponent->client_fd);
			observer_update(cur_game, "Draw \n");
			user->draw_match++;
			opponent->draw_match++;
        } else if (result == 4) {
            write_message(user->client_fd, "white win because black out of time \n");
			write_user_message_format(user, user->client_fd);
			write_message(opponent->client_fd, "white win because black out of time \n");
			write_user_message_format(opponent, opponent->client_fd);
			observer_update(cur_game, "white win because black out of time \n");
			if (piece == 1) {
				// user is white
				user->win_match++;
				opponent->loss_match++;
			} else {
				// user is black
				user->loss_match++;
				opponent->win_match++;
			}
        } else if (result == 5) {
            write_message(user->client_fd, "black win because white out of time \n");
			write_user_message_format(user, user->client_fd);
			write_message(opponent->client_fd, "black win because white out of time \n");
			write_user_message_format(opponent, opponent->client_fd);
			observer_update(cur_game, "black win because white out of time \n");
			if (piece == 1) {
				// user is white
				user->loss_match++;
				opponent->win_match++;
			} else {
				// user is black
				user->win_match++;
				opponent->loss_match++;
			}
        } else {
			write_user_message_format(opponent, opponent->client_fd);
			write_user_message_format(user, user->client_fd);
		}

		if (result > 0) {
			delete_game(cur_game);
		}
	}
	
}

/*
void observe_command(User *user, char *num) add observer to the game num 
*/
void observe_command(User *user, char *num) {
	int count = 0;
	Game *game_ptr = game_head;

	// check to see if game exist
	while (game_ptr != NULL) {
		if (count == atoi(num)) {
			break;
		}
		game_ptr = game_ptr->next;
	}

	if (game_ptr == NULL) {
		write_message(user->client_fd, "game not exist \n");
		write_user_message_format(user, user->client_fd);
		return;
	}

	// check to see if user already observe the game
	if (check_observer(game_ptr, user->username) == 0) {
		write_message(user->client_fd, "You are already observe the game \n");
		write_user_message_format(user, user->client_fd);
		return;
	} 

	// add observer
	add_observer(game_ptr, user);

	// print out the board
	write_message(user->client_fd, print_board(game_ptr));
	write_user_message_format(user, user->client_fd);
}

/*
void unobserve_command(User *user): unobserve game this user watch
*/
void unobserve_command(User *user) {
	Game *game_ptr = game_head;
	char *temp;
	int count = 0;

	temp = (char*) malloc(100*sizeof(char)); 
	while (game_ptr != NULL) {
		if (check_observer(game_ptr, user->username) == 0) {
			sprintf(temp, "Unobserving game %d\n", count);
			delete_observer(game_ptr, user);
			write_message(user->client_fd, temp);
			count++;
		}
		game_ptr = game_ptr->next;
	}

	if (count == 0) {
		write_message(user->client_fd, "You are not observing anything.\n");
	}
	write_user_message_format(user, user->client_fd);
	free(temp);
}

/*
void refresh_board(User *user): update board for current user
*/

void refresh_board(User *user) {
	Game *game_ptr;

	// player
	game_ptr = find_game(user->username); 
	if (game_ptr != NULL) {
		write_message(user->client_fd, print_board(game_ptr));
	}

	// observer
	game_ptr = game_head;
	while (game_ptr != NULL) {
		if (check_observer(game_ptr, user->username) == 0) {
			write_message(user->client_fd, print_board(game_ptr));
		}
		game_ptr = game_ptr->next;
	}

	write_user_message_format(user, user->client_fd);	
}

/*
void resign(User *user) player resign 
*/
void resign(User *user) {
	Game *game_ptr;
	char *temp;
	User *black, *white;


	temp = (char*) malloc(100*sizeof(char)); 
	// player
	game_ptr = find_game(user->username); 
	if (game_ptr != NULL) {
		black = find_user_with_name(game_ptr->black);
		white = find_user_with_name(game_ptr->white);
		sprintf(temp, "%s resigned\n", user->username);
		write_message(black->client_fd, temp);
		write_message(white->client_fd, temp);
		write_user_message_format(black, black->client_fd);	
		write_user_message_format(white, white->client_fd);	
		observer_update(game_ptr, temp);
		user->loss_match++;
		if (strcmp(black->username, user->username) == 0) {
			//change stats
			white->win_match++;
		} else {
			black->win_match++;
		}
		free(temp);
		// delete game
		delete_game(game_ptr);
	} else {
		write_message(user->client_fd, "You can't resign without playing a game.\n");
		write_user_message_format(user, user->client_fd);	
	}
}

/*
void process_mail_title(User *user, char **userInputs, int len): create temp mail
*/

void process_mail_title(User *user, char **userInputs, int len) {
	char *to, *title;

	to = userInputs[1];
	title = combineUserInputs(userInputs, len);
	create_temp_mail(user->username, to, title);
	user->status = 2; // writing email;
	write_message(user->client_fd, "\n\n\n");
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
	char *piece;
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
						if (found_user->status == 2) {
							if (strcmp(trimmedString, ".") == 0) {
								// end of mail, send messsage
								sendTempMail(found_user->username);

								// notify user that they have new mail
								write_message(client[i], "You have new email!\n");
								write_user_message_format(found_user, client[i]);

								// change status to 1
								found_user->status = 1;
							} else {
								add_message(found_user->username, trimmedString);
							}
						}
						else if (strcmp(userInput, "help") == 0 || strcmp(userInput, "?") == 0) {
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
						else if (strcmp(userInput, "shout") == 0) {
							// if the user only enters "shout"
							if (numWords == 1) {
								shout_command(found_user,  (char **)"", 0);
							}
							// if the user enters "shout ANY STRING"
							else {
								shout_command(found_user, userInputs, numWords);
							}

							write_user_message_format(found_user, client[i]);
						}
						else if (strcmp(userInput, "tell") == 0) {
							// if the user only enters "tell"
							if (numWords == 1) {
								tell_command(found_user, (char *)"", (char **)"", 0);
							}
							// if the user enters "tell USERNAME"
							else if (numWords == 2) {
								tell_command(found_user, userInputs[1], (char **)"", 1);
							}
							// if the user enters "tell USERNAME ANY STRING"
							else {
								char *userName = userInputs[1];
								removeFirstWords(userInputs, numWords, 1);
								tell_command(found_user, userName, userInputs, (numWords - 1));
							}

							write_user_message_format(found_user, client[i]);
						}
						else if (strcmp(userInput, "kibitz") == 0 || strcmp(userInput, "'") == 0) {
							// if the user only enters "shout"
							if (numWords == 1) {
								kibitz_command(found_user,  (char **)"", 0);
							}
							// if the user enters "shout ANY STRING"
							else {
								kibitz_command(found_user, userInputs, numWords);
							}

							write_user_message_format(found_user, client[i]);
						}
						else if (strcmp(userInput, "quiet") == 0) {
							change_quiet_command(found_user, 1);
							write_message(client[i], "Enter quiet mode.\n");
							write_user_message_format(found_user, client[i]);
						}
						else if (strcmp(userInput, "nonquiet") == 0) {
							change_quiet_command(found_user, 0);
							write_message(client[i], "Enter nonquiet mode.\n");
							write_user_message_format(found_user, client[i]);
						}
						else if (strcmp(userInput, "block") == 0) {
							// if the user only enters "block"
							if (numWords == 1) {
								block_command(found_user, "");
							}
							// if the user enters "block USERNAME", where anything after USERNAME is ignored
							else {
								block_command(found_user, userInputs[1]);
							}

							write_user_message_format(found_user, client[i]);
						}
						else if (strcmp(userInput, "unblock") == 0) {
							// if the user only enters "unblock"
							if (numWords == 1) {
								write_message(client[i], "Usage: unblock <id>\n");
							}
							// if the user enters "unblock USERNAME", where anything after USERNAME is ignored
							else {
								unblock_command(found_user, userInputs[1]);
							}

							write_user_message_format(found_user, client[i]);
						}
						// 
						else if (strcmp(userInput, "game") == 0) {
							// TODO: test after the game-related functions are implemented
							write_message(client[i], print_games());
							write_user_message_format(found_user, client[i]);
						}
						else if (strcmp(userInput, "match") == 0) {
							// match command
							if (numWords != 2 && numWords != 3 && numWords != 4) {
								write_message(client[i], "match <name> <b|w> [t]\n");
								write_user_message_format(found_user, client[i]);
							} else {
								game_ptr = find_game(found_user->username);
								if (game_ptr != NULL) {
									// a game under player name is currently in play
									write_message(client[i], "Please finish a game before starting a new one \n");
									write_user_message_format(found_user, client[i]);
									continue;
								}
								opponent = find_user_with_name(userInputs[1]); 
								if (opponent == NULL) {
									write_message(client[i], "User does not exist!\n");
									write_user_message_format(found_user, client[i]);
								} else if (strcmp(opponent->username, found_user->username) == 0) {
									write_message(client[i], "You cannot have a match with yourself.\n");
									write_user_message_format(found_user, client[i]);
								} else if (opponent->status == 0) {
									write_message(client[i], "User is not online\n");
									write_user_message_format(found_user, client[i]);
								} else {
									// TODO: check if the last argument is number or not
									time = 600;
									if (numWords == 4) {
										time = atoi(userInputs[3]);
									} 
									printf("DEBUG: time %d\n ", time);
									printf("DEBUG: userInputs[2] %s\n ", userInputs[2]);
									if (userInputs[2] == NULL) {
										piece = "b";
									}
									else {
										piece = (strcmp(userInputs[2], "b") != 0 && strcmp(userInputs[2], "w") != 0) ? "b" : userInputs[2];
									}
									// check if request exist
									check_flag = check_request(found_user->username, opponent->username, piece, time);
									printf("DEBUG: check_flag %d\n ", check_flag);
									if (check_flag == 0) {
										// new request
										create_request(found_user->username, opponent->username, piece, time);
										// let other player know 
										if (strcmp(piece, "w") == 0) {
											piece = "b";
										} else {
											piece = "w";
										}

										sprintf(temp, "%s invite you for a game <match %s %s %d>\n", found_user->username, found_user->username, piece, time);
										write_message(opponent->client_fd, temp);
										write_user_message_format(found_user, found_user->client_fd);
										write_user_message_format(opponent, opponent->client_fd);
									} else if (check_flag == 1) {
										// request exist and all information match
										printf("Debug: before delete request\n");
										// delete request
										delete_request_user(opponent->username);
										delete_request_user(found_user->username);
										printf("Debug: After delete request\n");
										// create game
										if (strcmp(piece, "b") == 0) {
											// create game
											game_ptr = create_game(found_user->username, opponent->username, time);
											// get board
											user_temp_str = print_board(game_ptr);
											if (user_temp_str == NULL) {
												printf("Error: not found board!");
												return 1;
											}
											
										} else {
											// create game
											game_ptr = create_game(opponent->username, found_user->username, time);
											// get board
											user_temp_str = print_board(game_ptr);
											if (user_temp_str == NULL) {
												printf("Error: not found board!");
												return 1;
											}
										}

										// send board to both user
										write_message(opponent->client_fd, user_temp_str);
										write_message(found_user->client_fd, user_temp_str);
										write_user_message_format(opponent, opponent->client_fd);
										write_user_message_format(found_user, found_user->client_fd);
										// free the board string
										free(user_temp_str);

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
						else if ((userInput[0] == 'A' || userInput[0] == 'B' || userInput[0] == 'C') && (userInput[1] == '1' || userInput[1] == '2' || userInput[1] == '3')) {
							// move command
							move_game(found_user, userInput);
						}
						else if (strcmp(userInput, "observe") == 0) {
							if (numWords == 1) {
								observe_command(found_user, "0");
							}
							else {
								observe_command(found_user, userInputs[1]);
							}
						}
						else if (strcmp(userInput, "unobserve") == 0) {
							// unobserve command
							unobserve_command(found_user);
						}
						else if (strcmp(userInput, "refresh") == 0) {
							// refresh
							refresh_board(found_user);
						}
						else if (strcmp(userInput, "resign") == 0) {
							// resign
							resign(found_user);
						}
						else if (strcmp(userInput, "mail") == 0) {
							// mail
							if (numWords < 3) {
								write_message(client[i], "invalid command, should look like this 'mail <id> <title>'\n");
								write_user_message_format(found_user, client[i]);
							} else {
								process_mail_title(found_user, userInputs, numWords - 1);
							}
						}
						else if (strcmp(userInput, "listmail") == 0) {
							// list mail
							user_temp_str = listmail(found_user->username);
							write_message(client[i], user_temp_str);
							write_user_message_format(found_user, client[i]);
							free(user_temp_str);
						} 
						else if (strcmp(userInput, "readmail") == 0) {
							// read mail
							// check to see if num legit
							user_temp_str = userInputs[1];
							check_flag = 0;
							while (*user_temp_str != '\0') {
								if (*user_temp_str < '0' || *user_temp_str > '9') {
									write_message(found_user->client_fd, "invalid command! \n");
									write_user_message_format(found_user, found_user->client_fd);
									check_flag = 1;
									break;
								}
								user_temp_str++;
							}
							// read mail
							if (check_flag == 0) {
								user_temp_str = readmail(found_user->username, atoi(userInputs[1]));
								write_message(client[i], user_temp_str);
								write_user_message_format(found_user, client[i]);
								free(user_temp_str);
							}
						}
						else if (strcmp(userInput, "deletemail") == 0) {
							// delete mail
							// check to see if num legit
							user_temp_str = userInputs[1];
							check_flag = 0;
							while (*user_temp_str != '\0') {
								if (*user_temp_str < '0' || *user_temp_str > '9') {
									write_message(found_user->client_fd, "invalid command! \n");
									write_user_message_format(found_user, found_user->client_fd);
									check_flag = 1;
									break;
								}
								user_temp_str++;
							}
							// delete mail
							if (check_flag == 0) {
								user_temp_str = deletemail(found_user->username, atoi(userInputs[1]));
								write_message(client[i], user_temp_str);
								write_user_message_format(found_user, client[i]);
								free(user_temp_str);
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
