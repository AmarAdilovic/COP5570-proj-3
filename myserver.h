#ifndef _MYSERVER_H
#define _MYSERVER_H
#include <stdio.h>
#include <time.h>

/* Typedefs */
typedef struct block_block {
    char *username;
    struct block_block *next;
} Blocked_user;

typedef struct mail_block {
    char *username; /* name of sender */
    int status; /* 0 for not read, 1 for read */
    char *title; /* title of the mail */
    char* message; /* message inside mail */
    time_t date; /* send date */
    struct mail_block *next;
} Mail;

typedef struct user_block {
    char *username; 
    char *password;
    char *info;
    int client_fd;
    int win_match; /* number of win matchs */
    int loss_match; /* number of loss matchs */
    int draw_match; /* number of draw matchs */
    int status; /* 0 is offline, 1 is online, 2 is waiting on password */
    Blocked_user* block_head; /* head of linked list of block users */
    Mail* mail_head; /* head of linked list of mails to user */
    int message_num; /* count the current message number */
    struct user_block *next;
} User;

typedef struct observer_block {
    User *user; /* pointer to user currently observe */
    struct observer_block *next;
} Observer;

typedef struct game_block {
    int board[3][3]; /* 0 is none, 1 is white, 2 is black */ 
    char* black; /* username of black */
    char* white; /* username of white */
    time_t last_time_white; // Last track time of white
    time_t last_time_black; // Last track time of black
    time_t time_left_white; // Time limit left for white
    time_t time_left_black; // Time limit left for black
    Observer *observer_head; /* head of linked list of observers of the current game */
    struct game_block *next;
} Game;

/* Global */
extern User *user_head;
extern Game *game_head;

/* Flag related */


/* prototypes from game.c */
int who_move(Game *);
Game *find_game(char *, char *);
Game *create_game(char *, char *, int);
void delete_game(Game *);
int add_observer(Game *, User *);
char *print_games(void);
void delete_observer(Game *, User *);
int move(char *, Game *);
char *print_board(Game *);
int isWin(Game *);


/* prototypes from myserver.c */

/* prototypes from mycommands.c */
char *help_command(void);
void change_password_command(User *, char *);

/* prototypes from messages.c */
char *initial_messsage(void);
char *welcome_message(void);
char *login_failed_message(void);
char *connection_closed_message(void);
char *guest_user_message(void);
char *guest_user_warning_message(void);

/* prototypes from backup.c */
void serialize(char *);
void deserialize(char *);


#endif
