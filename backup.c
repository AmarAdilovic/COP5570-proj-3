/*
File: backup.c
Purpose: contains function to store account information to a file 
and should be called from myserver.c every 5 minutes using alarm 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myserver.h"

/*
void serialize_block(FILE *file, Blocked_user *cur): save block users to given file
${block_users}$
Block_users: name, 
*/
void serialize_block(FILE *file, Blocked_user *cur) {
    fprintf(file, "${");
    for (cur; cur != NULL; cur = cur->next) {
        // save blocked_user username
        fprintf(file, "%s, ", cur->username);
    }
    fprintf(file, "}$");
}

/*
void serialize_mail(FILE *file, Mail *cur): save mail to given file
${mails}$
mails: ^(name, status, title, message)^,  
*/
void serialize_mail(FILE *file, Mail *cur) {
    fprintf(file, "${");
    for (cur; cur != NULL; cur = cur->next) {
        // save sender username
        fprintf(file, "^(%s, ", cur->username);
        // save mail status
        fprintf(file, "%d, ", cur->status);
        // save mail title
        fprintf(file, "%s, ", cur->title);
        // save mail message
        fprintf(file, "%s)^, ", cur->message);
    }
    fprintf(file, "}$");
}

/*
void serialize(): save the accounts information to a file given the file name as argument
Format to save account information to file:
Users: $(name, pwd, win_match, loss_match, draw_match,  ${block_users}$, ${mails}$)$, 
Block_users: name, 
mails: ^(name, status, title, message)^, 
TODO: call this function from myserver.c every 5 minutes
*/
void serialize(char *file_name) {
    User *cur = user_head;

    // Save acoount information to accounts.txt file
    FILE* file = fopen(file_name, "w");
    if (file == NULL) {
        exit(1);
    }

    for (cur; cur != NULL; cur = cur->next) {
        // open object and save username
        fprintf(file, "$(%s, ", cur->username);
        // save password
        fprintf(file, "%s, ", cur->password);
        // save win_match
        fprintf(file, "%d, ", cur->win_match);
        // save loss_match
        fprintf(file, "%d, ", cur->loss_match);
        // save draw_match
        fprintf(file, "%d, ", cur->draw_match);
        // save block_user
        serialize_block(file, cur->block_head);
        // save mail
        serialize_mail(file, cur->mail_head);
        // close linked_list object
        fprintf(file, ")$, ");
    }
    fclose(file);
}

