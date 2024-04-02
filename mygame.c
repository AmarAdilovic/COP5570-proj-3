/*
File: mygame.c
Purpose: contains all tic tac toe related functions
*/

#include "myserver.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/*
void refresh_time(Game *ptr): refresh the timing of the game of player based on current time
*/
void refresh_time(Game *ptr) {
    int color;
    time_t current, time_use;
    color = who_move(ptr);
    time(&current); /* get the current time */
    if (color == 1) {
        time_use = current - ptr->last_time_white; // calculate how many seconds passed
        ptr->time_left_white = ptr->time_left_white - time_use; // deduct it from time left
        ptr->last_time_white = current; // set the time of white to current time
    } else {
        time_use = current - ptr->last_time_black; // calculate how many seconds passed
        ptr->time_left_black = ptr->time_left_black - time_use; // deduct it from time left
        ptr->last_time_black = current; // set the time of black to current time
    }
}

/*
Game *find_game(char *user1, char *user2): find the existing game with given username of user1 and user2 as 
argument. Return the pointer to the game and return NULL if not found
*/

Game *find_game(char *user1, char *user2) {
    // pointer to head
    Game *ptr = game_head;
    // Find the game
    while (ptr != NULL) {
        if ((strcmp(ptr->black, user1) == 0 && strcmp(ptr->white, user2)) == 0 || (strcmp(ptr->black, user2) == 0 && strcmp(ptr->white, user1) == 0))
            return ptr;
        ptr = ptr->next;
    }   
    return NULL;
}

/*
int who_move(Game *ptr) this function will return whose move is next. 1 is white and 2 is black.
*/
int who_move(Game *ptr) {
    int i, j, color, count;
    count = 0;
    // decide which color (black or white) will make the next move
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (ptr->board[i][j] != 0)
                count++;
        }
    }

    // if count is divisible by 2, next move is black
    color = 1;
    if (count % 2 == 0) {
        color = 2;
    } 
    return color;
}

/*
Game *create_game(char *black, char *white): create a new game with given username for black and username for white
and time limit as argument and return the address pointer to the new game struct
*/

Game *create_game(char *black, char *white, int limit) {
    // pointer to pointer of head
    Game **ptr_ptr = &game_head;
    // pointer to head
    Game *ptr = game_head;
    int i, j;

    // find a place to add new game
    while (ptr != NULL) {
        ptr_ptr = &(ptr->next);
        ptr = ptr->next;
    }

    // allocate memory for new Game/match
    ptr = malloc(sizeof(Game));
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory when using create_game function\n");
        return NULL;
    }

    // set usernames for black and white fields
    ptr->black = strdup(black);
    if (ptr->black == NULL) {
        fprintf(stderr, "Out of memory when using create_game function\n");
        free(ptr);
        return NULL;
    }

    ptr->white = strdup(white);
    if (ptr->white == NULL) {
        fprintf(stderr, "Out of memory when using create_game function\n");
        free(ptr);
        return NULL;
    }

    // initialize board
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            ptr->board[i][j] = 0;
        }
    }

    time(&(ptr->last_time_black));
    time(&(ptr->last_time_white));
    ptr->time_left_white = limit;
    ptr->time_left_black = limit;
    ptr->observer_head = NULL;
    ptr->next = NULL;
    *ptr_ptr = ptr; /* This can help handle NULL(empty linked list) case */
    return ptr;
}

/*
void delete_game(Game *ptr): This function will delete game from list of game (for resign command or when the game finish)
*/
void delete_game(Game *ptr) {
    // pointer to head
    Game *cur = game_head;
    // pointer to observer of the game
    Observer *cur_observer = ptr->observer_head;
    Observer *cur_observer2 = ptr->observer_head;

    // if the game we want to delete is at the beginning of the list
    if (cur == ptr) {
        game_head = ptr->next;
    } else {
        while (cur->next != ptr) {
            cur = cur->next;
            if (cur == NULL) {
                fprintf(stderr, "Game not exist -- error in delete_game\n");
                return;
            }
        }
        cur->next = ptr->next;
    }

    // free allocated memory in ptr
    while (cur_observer != NULL) {
        cur_observer = cur_observer2->next;
        free(cur_observer2);
        cur_observer2 = cur_observer;
    }
    free(ptr->black);
    free(ptr->white);
}

/*
int count_observers(Game *ptr): count number of observer in the game
*/
int count_observers(Game *ptr) {
    int count = 0;
    Observer *cur = ptr->observer_head;
    while (cur != NULL) {
        cur = cur->next;
        count++;
    }
    return count;
}

/*
int count_moves(Game *ptr): count the number of moves has made in the game
*/
int count_moves(Game *ptr) {
    int i, j, count;
    count = 0;
    // decide which color (black or white) will make the next move
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (ptr->board[i][j] != 0)
                count++;
        }
    }
    return count;
}

/*
char *print_games(Game *ptr): print all the current games happening  
*/

char *print_games() {
    int count = 0;
    char *ret_val, *temp;
    Game *ptr = game_head;

    // Allocate memory for ret_val
    ret_val = (char*) malloc(1200*sizeof(char)); /* Remember to free ret_val after use */
    temp = (char*) malloc(50*sizeof(char));

    // Count total game
    while (ptr != NULL) {
        count++;
        ptr = ptr->next;
    }

    sprintf(ret_val, "Totals %d game(s):\n", count);

    // Print details of each game
    ptr = game_head;
    count = 0;
    while (ptr != NULL) {
        sprintf(temp, "Game %d(%d): %s  .vs.  %s, %d moves\n", count, count_observers(ptr), ptr->black, ptr->white, count_moves(ptr));
        strcat(ret_val, temp);
        count++;
        ptr = ptr->next;
    }

    
    free(temp);
    return ret_val;
}

/*
int add_observer(Game *game, User *user): add observer to the game given as argument, return 0 if success, 1 otherwise
use for observe command
*/
int add_observer(Game *game, User *user) {
    // pointer to pointer of head
    Observer **ptr_ptr = &game->observer_head;
    // pointer to head
    Observer *ptr = game->observer_head;

    // find a place to add new observer
    while (ptr != NULL) {
        ptr_ptr = &(ptr->next);
        ptr = ptr->next;
    }

    // allocate memory for new observer
    ptr = malloc(sizeof(Observer));
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory when using add_observer function\n");
        return 1;
    }

    ptr->user = user;
    ptr->next = NULL;
    *ptr_ptr = ptr;
    return 0;
}

/*
void delete_observer(Game *game, User *user): delete observer from the game 
NOTE: before using this function, MUST check that user is an observer in the game
use for unobserve command
*/
void delete_observer(Game *game, User *user) {
    // pointer to head
    Observer *ptr = game->observer_head;
    Observer *cur;
    cur = ptr;

    // if the game we want to delete is at the beginning of the list
    if (user == ptr->user) {
        game->observer_head = ptr->next;
    } else {
        while (ptr->next != NULL && ptr->next->user != user) {
            ptr = ptr->next;
        }
        if (ptr->next == NULL) {
            fprintf(stderr, "User not exist in observer list -- error in delete_observer\n");
            return;
        }
        cur = ptr->next;
        ptr->next = cur->next;
    }
    free(cur);
}

/*
int isWin(Game *ptr): check whether the game is finish or not given the pointer to game object as an argument. 
Return 0 is the game is not finish, 1 if white win, 2 if black win, 3 if draw, 4 if white win because black 
out of time, 5 if black win because white out of time.
*/

int isWin(Game *ptr) {
    int i, j;

    // check time
    refresh_time(ptr);
    if (ptr->time_left_white <= 0)
        return 5; // black win because white out of time.
    if (ptr->time_left_black <= 0)
        return 4; // white win because black out of time

    // check horizontally and vertically
    for (i = 0; i < 3; i++) {
        // check horizontal
        if (ptr->board[i][0] == ptr->board[i][1] && ptr->board[i][1] == ptr->board[i][2] && ptr->board[i][0] != 0) {
            return ptr->board[i][0];
        } 
        // check vertical
        if (ptr->board[0][i] == ptr->board[1][i] && ptr->board[1][i] == ptr->board[2][i] && ptr->board[0][i] != 0) {
            return ptr->board[0][i];
        } 
    }

    // check diagonally
    if (ptr->board[0][0] == ptr->board[1][1] && ptr->board[1][1] == ptr->board[2][2] && ptr->board[0][0] != 0) {
            return ptr->board[0][0];
    } 

    if (ptr->board[2][0] == ptr->board[1][1] && ptr->board[1][1] == ptr->board[0][2] && ptr->board[1][1] != 0) {
            return ptr->board[1][1];
    } 

    // check for draw
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (ptr->board[i][j] == 0)
                return 0;
        }
    }

    // draw
    return 3;
}

/*
int move(char *cor, Game *ptr): This function will make the move in the board given the coordinate as char and game pointer as arguments. 
The function will decide black or white move based on the rule "black always making the first move". Function will return 0 if the move is
success and the game is not finish, -1 if the move fail because the place is taken, -2 if the move fail because of not correct coordinate 
format given, 1 if white win, 2 if black win, 3 if draw, 4 if white win because black out of time, 5 if black win because white out of time.
*/

int move(char *cor, Game *ptr) {
    int i, j, color, count;
    time_t current;
    count = 0;
    // decide which color (black or white) will make the next move
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (ptr->board[i][j] != 0)
                count++;
        }
    }

    // if count is divisible by 2, next move is black
    color = 1;
    if (count % 2 == 0) {
        color = 2;
    } 

    // check to see if the given coordinate is valid
    if (strlen(cor) != 2)
        return -2;
    // check first character
    if (*cor != 'A' && *cor != 'B' && *cor != 'C') {
        return -2;
    } 
    // check second character
    if (*(cor+1) != '1' && *(cor+1) != '2' && *(cor+1) != '3') {
        return -2;
    } 

    // check to see if the place is already taken or not
    if (ptr->board[*cor - 'A'][*(cor+1) - '1'] != 0)
        return -1;

    // set the time stamp in the game and deduct the time from time left
    refresh_time(ptr);
    time(&current); /* get the current time */
    if (color == 1) {
        ptr->last_time_black = current; // set the timer for black
    } else {
        ptr->last_time_white = current; // set the timer for white
    }

    // set the position to the color
    ptr->board[*cor - 'A'][*(cor+1) - '1'] = color;

    // return status of the board (win, draw, not finish)
    return isWin(ptr);
    
} 

/*
char *print_board(Game *ptr) This function will print out the board and return the string contains the new board
*/
char *print_board(Game *ptr) {
    int i, j, size, count, ret_val_size;
    char row;
    size = 45;
    ret_val_size = 160;
    char *ret_val, *str_ret_val;

    // reset_time
    refresh_time(ptr);
    
    // Print stat
    str_ret_val = (char*) malloc(ret_val_size*sizeof(char));
    sprintf(str_ret_val, "Black:%15s       White:%15s\n Time:%15d        Time:%15d\n\n", ptr->black, ptr->white, (int) ptr->time_left_black, (int) ptr->time_left_white);
    /* TODO remember to free() the string after printout to client */

    // Print board
    ret_val = (char*) malloc(size*sizeof(char)); 
    if (ret_val == NULL) {
        fprintf(stderr, "Out of memory when using print_board function\n");
        return NULL;
    }

    // initialize return string
    for (i = 0; i < size; i++) {
        ret_val[i] = ' ';
    }

    ret_val[3] = '1';
    ret_val[6] = '2';
    ret_val[9] = '3';
    ret_val[10] = '\n';
    count = 11;
    row = 'A';
    for (i = 0; i < 3; i++) {
        ret_val[count] = row;
        // increase row A -> B, B -> C
        row = row + 1;
        count = count + 3;
        for (j = 0; j < 3; j++) {
            if (ptr->board[i][j] == 0)
                ret_val[count] = '.';
            else if (ptr->board[i][j] == 1)
                ret_val[count] = 'O';
            else
                ret_val[count] = '#';
 
            if (j != 2) 
                count += 3;
            else 
                count += 1;
        }
        if (i != 2)
            ret_val[count++] = '\n';
        else
            ret_val[count] = '\0'; /* end of string */
    }

    // concat stat and board
    strcat(str_ret_val, ret_val);
    free(ret_val);

    return str_ret_val;
}

