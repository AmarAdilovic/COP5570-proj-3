/*
File: mymail.c
Purpose: contains all mail related functions
#listmail                # List the header of the mails\n"
"#readmail <msg_num>      # Read the particular mail\n"
"#deletemail <msg_num>    # Delete the particular mail\n"
"#mail <id> <title>       # Send id a mail\n"
*/

#include "myserver.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/*
char *listmail(char *username): list all mail the user have
*/

char *listmail(char *username) {
    char s;
    char *ret_val, *temp;
    User *user;
    Mail *cur;
    int count;
    struct tm * timeinfo;
    count = 0;

    ret_val = (char*) malloc(1200*sizeof(char));
    temp = (char*) malloc(100*sizeof(char));
    s = 'N';

    user = find_user_with_name(username);
    cur = user->mail_head;

    // check if the mail box is empty
    if (cur == NULL) {
        // The mail box is empty
        sprintf(ret_val, "You have no messages.\n");
        
    } else {
        // The mail box contains mails
        sprintf(ret_val, "Your messages:\n");
        timeinfo = localtime(&cur->date);
        for (; cur != NULL; cur = cur->next) {
            if (cur->status == 0) {
                s = 'N';
            } else {
                s = 'R';
            }
            sprintf(temp, "%3d%3c%12s%43s\n", count, s, cur->username, cur->title, asctime (timeinfo));
            strcat(ret_val, temp);
            count++;
        }
    }

    free(temp);
    return ret_val;
}

/*
char *readmail(char *username, int num): read the #num mail of given username and return the result
*/
char *readmail(char *username, int num) {
    int count;
    char *ret_val;
    User *user;
    Mail *cur;
    struct tm * timeinfo;

    ret_val = (char*) malloc(1200*sizeof(char));
    user = find_user_with_name(username);
    cur = user->mail_head;
    count = 0;

    // find email
    while (cur != NULL) {
        if (count == num) {
            timeinfo = localtime(&cur->date);
            sprintf(ret_val, "From: %s\nTitle: %s\nTime: %s\n\n%s", cur->username, cur->title, asctime (timeinfo), cur->message);
            return ret_val;
        }
        count++;
        cur = cur->next;
    }

    sprintf(ret_val, "Message number invalid\n");
    return ret_val;
}

/*
char *deletemail(char *username, int num): delete the #num mail of given username and return the message to response to user
*/
int deletemail(char *username, int num) {
    int count;
    char *ret_val;
    User *user;
    Mail *cur, *ptr;

    user = find_user_with_name(username);
    ret_val = (char*) malloc(100*sizeof(char));
    cur = user->mail_head;
    count = 0;

    if (cur == NULL) {
        // empty mail box
        sprintf(ret_val, "Message number invalid\n");
        return ret_val;
    }

    if (count == num) {
        // the mail head is the mail the user want to delete
        user->mail_head = cur->next;

        // free string
        free(cur->message);
        free(cur->title);
        free(cur);

        sprintf(ret_val, "Message deleted\n");
        return ret_val;
    }

    // find email
    while (cur != NULL) {
        if (count == num) {
            ptr->next = cur->next;
            // free string
            free(cur->message);
            free(cur->title);
            free(cur->username);
            free(cur);
            sprintf(ret_val, "Message deleted\n");
            return ret_val;
        }
        count++;
        ptr = cur;
        cur = cur->next;
    }

    // Not found
    sprintf(ret_val, "Message number invalid\n");
    return ret_val;
}

/*
int createmail(char *username, char *from, char *title, char *message): create a new email and add that to usename mailbox
TODO: work on notify user about new email after create a new email. Return 0 if success, 1 otherwise
*/
int createmail(char *username, char *from, char *title, char *message) {
    User *user;
    Mail *ptr;
    Mail **ptr_ptr;


    user = find_user_with_name(username);
    ptr = user->mail_head; // pointer to head
    ptr_ptr = &user->mail_head; // pointer to pointer of head

    // find a place to add new mail
    while (ptr != NULL) {
        ptr_ptr = &(ptr->next);
        ptr = ptr->next;
    }

    // Allocate memory for new mail
    ptr = malloc(sizeof(Mail));
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory when using createmail function\n");
        return 1;
    }

    ptr->username = strdup(from);
    ptr->title = strdup(title);
    if (ptr->title == NULL) {
        fprintf(stderr, "Out of memory in the createmail function during from setup\n");
        free(ptr);
        return NULL;
    }

    ptr->title = strdup(title);
    if (ptr->title == NULL) {
        fprintf(stderr, "Out of memory in the createmail function during title setup\n");
        free(ptr);
        return NULL;
    }

    ptr->message = strdup(message);
    if (ptr->title == NULL) {
        fprintf(stderr, "Out of memory in the createmail function during message setup\n");
        free(ptr);
        return NULL;
    }

    // set the send time
    time(&(ptr->date));
    // set status to not read or 0
    ptr->status = 0;
    ptr->next = NULL;
    *ptr_ptr = ptr;
    return 0;
}



