#ifndef _MYSERVER_H
#define _MYSERVER_H
#include <stdio.h>
#include <time.h>

// user statuses
#define USER_OFFLINE_STATUS 0
#define USER_ONLINE_STATUS 1

#define TEMP_USER_GUEST_STATUS 1
#define TEMP_USER_ACTIVE_CONNECTION_PENDING_STATUS 2
#define TEMP_USER_WAITING_ON_PASSWORD_STATUS 3
#define TEMP_USER_WAITING_PASSWORD_NOT_FOUND_STATUS 4

/* Typedefs */
typedef struct block_block {
    char *username;
    struct block_block *next;
} BlockedUser;

typedef struct mail_block {
    char *username; /* name of sender */
    int status; /* 0 for not read, 1 for read */
    char *title; /* title of the mail */
    char* message; /* message inside mail */
    time_t date; /* send date */
    struct mail_block *next;
} Mail;

typedef struct temp_mail_block {
    char *to;
    char *from;
    char *title;
    char *message;
    struct temp_mail_block *next;
} TempMail;

typedef struct user_block {
    char *username; 
    char *password;
    char *info;
    int client_fd;
    int win_match; /* number of win matchs */
    int loss_match; /* number of loss matchs */
    int draw_match; /* number of draw matchs */
    /*
    0 is offline,
    1 is online (they have an active connection to the server),
    */
    int status;
    /*
    0 is nonquiet (default value),
    1 is quiet mode,
    2 is writting email
    */
    int quiet;
    BlockedUser* block_head; /* head of linked list of block users */
    Mail* mail_head; /* head of linked list of mails to user */
    int message_num; /* count the current message number */
    struct user_block *next;
} User;

typedef struct temp_user_block {
    char *username; 
    char *password;
    int client_fd;
    /*
    0 is the initial state
    1 is a guest user
    2 is waiting of password but there is an active connection on this account,
    3 is waiting on password,
    4 is waiting on password but user not found
    */
    int status;
    int message_num; /* count the current message number */
    struct temp_user_block *next;
} TempUser;

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

typedef struct game_request {
    char *from;
    char *to;
    char *bw;
    int time;
    struct game_request *next;
} Request;

/* Global */
extern User *user_head;
extern Game *game_head;
extern TempUser *temp_user_head;
extern TempMail *temp_mail_head;
extern Request *request_head;

/* Flag related */


/* prototypes from game.c */
int who_move(Game *);
Game *find_game(char *);
Game *create_game(char *, char *, int);
void delete_game(Game *);
int add_observer(Game *, User *);
char *print_games(void);
void delete_observer(Game *, User *);
int move(char *, Game *);
char *print_board(Game *);
int isWin(Game *);
int check_observer(Game *, char *);


/* prototypes from myserver.c */
User *create_user(char *, int);
User *find_user_with_name(char *);
void write_message(int, char *);
void write_temp_user_message_format(TempUser *, int);
char* combineUserInputs(char**, int);
int count_online_users();
void removeFirstWords(char **, int, int);
int create_blocked_user(char *, User *);
BlockedUser *find_blocked_user_with_name(char *, BlockedUser *);
void delete_blocked_user(char *, User *);

/* prototypes from mycommands.c */
char *help_command(void);
void register_command(int, TempUser *, char *, char *);
void change_password_command(User *, char *);
void stats_command(int, char *);
void info_command(User *, char**, int);
void who_command(int);
void shout_command(User *, char**, int);
void tell_command(User *,  char *, char **, int);
void change_quiet_command(User *, int);
void block_command(User *, char *);
void unblock_command(User *, char *);
void kibitz_command(User *, char**, int);

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

/* prototypes from mymail.c */
char *listmail(char *);
char *readmail(char *, int);
char *deletemail(char *, int);
int createmail(char *, char *, char *, char *);
TempMail *create_temp_mail(char *, char *, char *);
int add_message(char *, char *);
int sendTempMail(char *);

/* prototypes from myrequest.c */
int delete_request(char *, char *);
int create_request(char *, char *, char *, int );
int check_request(char *, char *, char *, int);
char *get_request(char *, char *);
int delete_request_user(char *);
Request *find_request(char *, char *);

#endif
