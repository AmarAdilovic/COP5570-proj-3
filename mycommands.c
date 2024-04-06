/*
File: mycommands.c
Purpose: contains all commands a user can instruct the server to call, 
also called by the server in certain senarios (such as on user login)
*/

#include "myserver.h"
// for stdup
#include <string.h>
// for free
#include <stdlib.h>

// this returns the help menu which lists the commands available to the user
char *help_command() {
    // TODO: the pound symbol "#" infront of each command is for the commands we do not have implemented,
    // remove them as we implement the commands
    char* RETURNED_STRING = 
    "\nCommands supported:\n"
    "  who                     # List all online users\n"
    "  stats [name]            # Display user information\n"
    "  game                    # list all current games\n"
    "  observe <game_num>      # Observe a game\n"
    "  unobserve               # Unobserve a game\n"
    "  match <name> <b|w> [t]  # Try to start a game\n"
    "  <A|B|C><1|2|3>          # Make a move in a game\n"
    "  resign                  # Resign a game\n"
    "  refresh                 # Refresh a game\n"
    "  shout <msg>             # shout <msg> to every one online\n"
    "  tell <name> <msg>       # tell user <name> message\n"
    "  kibitz <msg>            # Comment on a game when observing\n"
    "  #' <msg>                 # Comment on a game\n"
    "  quiet                   # Quiet mode, no broadcast messages\n"
    "  nonquiet                # Non-quiet mode\n"
    "  block <id>              # No more communication from <id>\n"
    "  unblock <id>            # Allow communication from <id>\n"
    "  listmail                # List the header of the mails\n"
    "  readmail <msg_num>      # Read the particular mail\n"
    "  deletemail <msg_num>    # Delete the particular mail\n"
    "  mail <id> <title>       # Send id a mail\n"
    "  info <msg>              # change your information to <msg>\n"
    "  passwd <new>            # change password\n"
    "  exit                    # quit the system\n"
    "  quit                    # quit the system\n"
    "  help                    # print this message\n"
    "  ?                       # print this message\n";

    return RETURNED_STRING;
}

// this registers a given user
void register_command(int client_fd, TempUser *temp_user, char *username, char *password) {
    User *found_user_by_name = find_user_with_name(username);
    // if no user is found, we create the user
    if (found_user_by_name == NULL) {
        User *created_user = create_user(username, -1);
        change_password_command(created_user, password);
        write_message(client_fd, (char *)"User registered.\n");
        write_temp_user_message_format(temp_user, client_fd);
    }
    // if an existing user is found
    else {
        // username can only be 100 (size of buffer)
        char* message = (char*)malloc(100 + 50);

        if (message == NULL) {
            printf("Failed to allocate memory.\n");
            return;
        }

        // sprintf to write the formatted message to the allocated buffer
        sprintf(message, "%s has been registered. Please change the username.\n", found_user_by_name->username);

        write_message(client_fd, message);

        // free the allocated memory
        free(message);
    }
}

// returns a string that contains the blocked users for a given user
char *list_blocked_users(User *user) {
    // username can be 100 (maximum of 5 blocked users)
    // 50 for some buffer
    char* message = (char*)malloc((100 * 5) + 50);
    char* combinedOnlineUsernames = (char*)malloc((100 * 5) + 10);

    strcpy(message, "");
    strcpy(combinedOnlineUsernames, "");

    BlockedUser *ptr = user->block_head;
    if (ptr == NULL) {
        strcat(combinedOnlineUsernames, "<none>");
    }
    else {
        while (ptr != NULL) {
            strcat(combinedOnlineUsernames, ptr->username);
            strcat(combinedOnlineUsernames, " ");
            ptr = ptr->next;
        }
    }

    sprintf(
        message,
        "Blocked users: %s\n",
        combinedOnlineUsernames
        );

    return message;
}

// this shows a given users stats
void stats_command(int client_fd, char *username) {
    User *found_user_by_name = find_user_with_name(username);
    // no user is found
    if (found_user_by_name == NULL) {
        // username can only be 100 (size of buffer)
        char* message = (char*)malloc(100 + 30);

        if (message == NULL) {
            printf("Failed to allocate memory.\n");
            return;
        }

        // sprintf to write the formatted message to the allocated buffer
        sprintf(message, "User %s does not exist.\n", username);

        write_message(client_fd, message);
        free(message);
    }
    // if an existing user is found
    else {
        // username can be 100 (we call it twice)
        // info can be x
        // quiet value can be 3
        // 200 for the constant text (over-allocating)
        char* message = (char*)malloc(100 + 100 + 3 + 200);

        if (message == NULL) {
            printf("Failed to allocate memory.\n");
            return;
        }

        const char* info_message = (strcmp(found_user_by_name->info, "") == 0) ? "<none>" : found_user_by_name->info;
        const char* status_message = (found_user_by_name->status == 1) ? "currently online" : "off-line";
        const float rating = (found_user_by_name->win_match * 0.2) + (found_user_by_name->draw_match * 0.1);
        const char* quiet_message = (found_user_by_name->quiet == 0) ? "No" : "Yes";
        const char* blocked_user_message = list_blocked_users(found_user_by_name);

        // sprintf to write the formatted message to the allocated buffer
        sprintf(
            message,
            "User: %s\n"
            "Info: %s\n"
            "Rating: %f\n"
            "Wins: %d, Loses: %d\n"
            "Quiet: %s\n"
            "%s\n\n"
            "%s is %s.\n",
            username,
            info_message,
            rating,
            found_user_by_name->win_match,
            found_user_by_name->loss_match,
            quiet_message,
            blocked_user_message,
            username,
            status_message
            );

        write_message(client_fd, message);
        free(message);
    }
}

// this changes the info for a given user
void info_command(User *user, char** user_inputs, int num_words) {
    char* combined = combineUserInputs(user_inputs, num_words);
    user->info = strdup(combined);

    if (user->info == NULL) {
        fprintf(stderr, "Out of memory when trying to change the password.\n");
        return;
    }
}

// lists all online users
void who_command(int client_fd) {
    int numOnlineUsers = count_online_users();

    // username can be 100 (maximum of 20 users)
    // 50 for some buffer
    char* message = (char*)malloc((100 * numOnlineUsers) + 50);
    char* combinedOnlineUsernames = (char*)malloc((100 * numOnlineUsers) + 50);

    strcpy(message, "");
    strcpy(combinedOnlineUsernames, "");

    User *ptr = user_head;
    while (ptr != NULL) {
        if (ptr->status == USER_ONLINE_STATUS || ptr->status == USER_ONLINE_STATUS_MAIL) {
            strcat(combinedOnlineUsernames, ptr->username);
            strcat(combinedOnlineUsernames, " ");
        }
		ptr = ptr->next;
	}

    sprintf(
        message,
        "Total %d user(s) online:\n"
        "%s\n",
        numOnlineUsers,
        combinedOnlineUsernames
        );

    write_message(client_fd, message);
    free(message);
    free(combinedOnlineUsernames);
}

// this changes the password for a given user
void change_password_command(User *user, char *new_password) {
    user->password = strdup(new_password);
    if (user->password == NULL) {
        fprintf(stderr, "Out of memory when trying to change the password.\n");
        return;
    }
}

// sends a message to every online user from a specific user
// unless the user has quiet mode enabled or the user shouting has been blocked
void shout_command(User *user, char** user_inputs, int num_words) {
    // user input can only be 100
    // username can maximum be 100
    // 50 for some buffer
    char* message = (char*)malloc(100 + 100 + 50);
    char* combined = (num_words == 0) ? "\n" : combineUserInputs(user_inputs, num_words);

    strcpy(message, "");
    sprintf(
        message,
        "!shout! *%s*: %s\n",
        user->username,
        combined
        );

    User *ptr = user_head;
    while (ptr != NULL) {
        BlockedUser *found_blocked_user = find_blocked_user_with_name(user->username, ptr->block_head);
        if (ptr->status == USER_ONLINE_STATUS && ptr->quiet == 0 && found_blocked_user == NULL) {
            write_message(ptr->client_fd, message);
        }
		ptr = ptr->next;
	}
    free(message);
}

// sends a message to every observer in the game
// unless the user has quiet mode enabled or the user shouting has been blocked
void kibitz_command(User *user, char** user_inputs, int num_words) {
    // user input can only be 100
    // username can maximum be 100
    // 50 for some buffer
    Game *game_ptr = game_head;
    Observer *observer_ptr;
    User *ptr;
	int count = 0;

    char* message = (char*)malloc(100 + 100 + 50);
    char* combined = (num_words == 0) ? "\n" : combineUserInputs(user_inputs, num_words);

    strcpy(message, "");
    sprintf(
        message,
        "Kibitz* %s: %s\n",
        user->username,
        combined
        );

    for (; game_ptr != NULL; game_ptr = game_ptr->next) {
        if (check_observer(game_ptr, user->username) == 0) {
            count++;
            for (observer_ptr = game_ptr->observer_head; observer_ptr != NULL; observer_ptr = observer_ptr->next) {
                ptr = observer_ptr->user;
                BlockedUser *found_blocked_user = find_blocked_user_with_name(user->username, ptr->block_head);
                if (ptr->status == USER_ONLINE_STATUS && ptr->quiet == 0 && found_blocked_user == NULL) {
                    write_message(ptr->client_fd, message);
                }
            } 
        }
    }
    free(message);
    if (count == 0) {
		write_message(user->client_fd, "You are not observing any game.\n");
	}
}

// sends a message to a specific online user from a specific user, unless the specific user has been blocked
void tell_command(User *user, char *user_name, char** user_inputs, int num_words) {
    // user input can only be 100
    // username can maximum be 100
    // 50 for some buffer
    char* recepient_message = (char*)malloc(100 + 100 + 50);
    // username can maximum be 100
    // 50 for some buffer
    char* sender_message = (char*)malloc(100 + 50);
    char* combined = (num_words == 1) ? "\n" : combineUserInputs(user_inputs, num_words);

    strcpy(recepient_message, "");
    strcpy(sender_message, "");

    sprintf(
        recepient_message,
        "%s: %s\n",
        user->username,
        combined
        );

    User *ptr = user_head;
    int usersMessaged = 0;
    while (ptr != NULL) {
        if (strcmp(ptr->username, user_name) == 0 && ptr->status == USER_ONLINE_STATUS) {
            usersMessaged += 1;
            BlockedUser *found_blocked_user = find_blocked_user_with_name(user->username, ptr->block_head);

            if (found_blocked_user != NULL) {
                sprintf(
                    sender_message,
                    "You cannot talk to %s. You are blocked\n",
                    user_name
                    );
                write_message(user->client_fd, sender_message);
            }
            else {
                write_message(ptr->client_fd, recepient_message);
            }
        }
        else if (strcmp(ptr->username, user_name) == 0 && ptr->status == USER_OFFLINE_STATUS) {
            usersMessaged += 1;
            sprintf(
                sender_message,
                "User %s is not online.\n",
                user_name
                );
            write_message(user->client_fd, sender_message);
        }
		ptr = ptr->next;
	}
    if (usersMessaged == 0) {
        sprintf(
            sender_message,
            "User %s does not exist.\n",
            user_name
            );
        write_message(user->client_fd, sender_message);
    }

    free(recepient_message);
    free(sender_message);
}

// A player in the quiet mode does not receive any broadcast messages (from shout and kibitz), but
// should receive personal message (tell and mail).
// quiet == 0 is the default value
void change_quiet_command(User *user, int new_quiet_value) {
    user->quiet = new_quiet_value;
}

// blocks a specific username
void block_command(User *user, char *user_name) {
    if (strcmp(user_name, "") == 0) {
        write_message(user->client_fd, list_blocked_users(user));
    }
    else {
        // username can be 100
        // 20 for some buffer
        char* message = (char*)malloc(100 + 20);
        strcpy(message, "");

        BlockedUser *found_blocked_user = find_blocked_user_with_name(user_name, user->block_head);

        if (found_blocked_user != NULL) {
            sprintf(
                message,
                "%s was blocked before.\n",
                user_name
                );
        }
        else {
            int response = create_blocked_user(user_name, user);
            if (response == 0) {
                sprintf(
                    message,
                    "User %s blocked.\n",
                    user_name
                    );
            }
            else {
                sprintf(
                    message,
                    "Unable to block %s.\n",
                    user_name
                    );
            }

        }
        write_message(user->client_fd, message);
        free(message);
    }
}

// blocks a specific username
void unblock_command(User *user, char *user_name) {
    // username can be 100
    // 20 for some buffer
    char* message = (char*)malloc(100 + 20);
    strcpy(message, "");

    BlockedUser *found_blocked_user = find_blocked_user_with_name(user_name, user->block_head);

    if (found_blocked_user == NULL) {
        sprintf(
            message,
            "User %s was not blocked.\n",
            user_name
            );
    }
    else {
        delete_blocked_user(user_name, user);

        sprintf(
            message,
            "User %s unblocked.\n",
            user_name
            );

    }
    write_message(user->client_fd, message);
    free(message);
}
