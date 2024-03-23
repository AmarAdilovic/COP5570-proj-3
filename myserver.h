#ifndef _MYSERVER_H
#define _MYSERVER_H
#include <stdio.h>
#include <time.h>

//Typedefs
typedef struct game_block {
    int board[3][3]; // 0 is none, 1 is white, 2 is black 
    char* black; // username of black
    char* white; // username of white
    struct game_block *next;
} game;
 
typedef struct block_block {
    char *username;
    struct block_block *next;
} block;

typedef struct mail_block {
    char *username; // name of sender
    int *status; // 0 for not read, 1 for read
    int *title; // title of the mail
    int *message; // message inside mail
} mail;

typedef struct user_block {
    char *username; 
    char *password;
    int win_match; // number of win matchs
    int loss_match; // number of loss matchs
    int draw_match; // number of draw matchs
    int status; // 0 is offline, 1 is online
    block* block_head; // head of linked list of block users
    mail* mail_head; // head of linked list of mails to user
} user;

// Global 
extern user *user_head;
extern game *game;

// Flag related


//prototypes from game.c




#endif