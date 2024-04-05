/*
File: myrequest.c
Purpose: This file is used to handle game request
*/
#include "myserver.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* 
int delete_request_user(char *user): delete the request related to given user. Return 0 if success, 1 otherwise
*/
int delete_request_user(char *user) {
    Request *cur;
    Request *ptr = request_head;
    cur = ptr;

    // empty list
    if (ptr == NULL) {
        return 1;
    }

    // if request is in the beginning of the list
    if (strcmp(ptr->from, user) == 0 || strcmp(ptr->to, user) == 0) {
        request_head = request_head->next;
        free(cur->from);
        free(cur->to);
        free(cur);
        return delete_request_user(user);
    }

    while (ptr != NULL) {
        if (strcmp(ptr->from, user) == 0 || strcmp(ptr->to, user) == 0) {
            cur->next = ptr->next;
            free(ptr->from);
            free(ptr->to);
            free(ptr);
            return delete_request_user(user);
        }
        cur = ptr;
        ptr = ptr->next;
    }

    return 0;
}


/*
int delete_request(char *from, char *to): delete the request with from and to exit in 
linkedlist, 0 if delete success, 1 if request did not exist
*/
int delete_request(char *from, char *to) {
    Request *cur;
    Request *ptr = request_head;
    cur = ptr;

    // empty list
    if (ptr == NULL) {
        return 1;
    }

    // if request is in the beggining of the list
    if (strcmp(ptr->from, from) == 0 && strcmp(ptr->to, to) == 0) {
        request_head = request_head->next;
        free(cur->from);
        free(cur->to);
        free(cur);
        return 0;
    }

    while (ptr != NULL) {
        if (strcmp(ptr->from, from) == 0 && strcmp(ptr->to, to) == 0) {
            cur->next = ptr->next;
            free(ptr->from);
            free(ptr->to);
            free(ptr);
            return 0;
        }
        cur = ptr;
        ptr = ptr->next;
    }

    return 1;
}

/*
int create_request(char *from, char *to, char bw, int time): create a request for the game return 
0 if success, return 1 if the other player is offline, 2 if player is not exist, -1 otherwise
*/
int create_request(char *from, char *to, char *bw, int time) {
    printf("DEBUG: create_request %s %s %s %d\n ", from, to, bw, time);
    User *player2;
    Request *ptr = request_head;
    Request **ptr_ptr = &request_head;

    delete_request(from, to);

    player2 = find_user_with_name(to);
    if (player2 == NULL) {
        return 2;
    }

    if (player2->status == 0) {
        return 1;
    }

    // find a place to add new request
    while (ptr != NULL) {
        ptr_ptr = &(ptr->next);
        ptr = ptr->next;
    }

    // allocate memory for new user
    ptr = malloc(sizeof(Request));
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory when using create_request function\n");
        return -1;
    }

    // set field
    ptr->from = strdup(from);
    if (ptr->from == NULL) {
        fprintf(stderr, "Out of memory when using create_request function\n");
        return -1;
    }
    ptr->to = strdup(to);
    if (ptr->to == NULL) {
        fprintf(stderr, "Out of memory when using create_request function\n");
        return -1;
    }

    ptr->bw = strdup(bw);
    if (ptr->bw == NULL) {
        fprintf(stderr, "Out of memory when using create_request function\n");
        return -1;
    }

    ptr->time = time;

    ptr->next = NULL;
    *ptr_ptr = ptr;
    return 0;
}

/* 
int check_request(char *from, char *to, char bw, int time): check if request made on 
the other side. Return 0 if not, 1 if is all information are correct, 2 if information 
is not aligned
*/

int check_request(char *from, char *to, char *bw, int time) {
    Request *ptr = request_head;

    while (ptr != NULL) {
        if (strcmp(ptr->from, to) == 0 && strcmp(ptr->to, from) == 0) {
            if (strcmp(ptr->bw, bw) != 0 && ptr->time == time)
                return 1;
            else 
                return 2;
        }
        ptr = ptr->next;
    }

    return 0;

}

/*
Request *find_request(char *from, char *to): return the request with given from and to, NULL if not found
*/
Request *find_request(char *from, char *to) {
    Request *ptr = request_head;

    while (ptr != NULL) {
        if (strcmp(ptr->from, from) == 0 && strcmp(ptr->to, to) == 0) {
            return ptr;
        }
        ptr = ptr->next;
    }

    return NULL;
}

/*
char *get_request(char *from, char *to): Use in conflict request case return the format string with all the information, NULL
if the request did not exist 
*/
char *get_request(char *from, char *to) {
    Request *ptr = request_head;
    char *ret_val;

    ret_val = (char*) malloc(1200*sizeof(char));

    while (ptr != NULL) {
        if (strcmp(ptr->from, from) == 0 && strcmp(ptr->to, to) == 0) {
            sprintf(ret_val, "%s wants <match %s %s %d>", ptr->from, ptr->to, ptr->bw, ptr->time);
            return ret_val;
        }
        ptr = ptr->next;
    }

    return NULL;
}









