/*
File: backup.c
Purpose: contains function to store account information to a file 
and should be called from myserver.c every 5 minutes using alarm 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "myserver.h"


/*
char *encrypt(char *str): encrypt string into ascii and return the pointer to ascii string
*/
char *encrypt(char *str) {
    char *ret_val, *temp;
    int i, len;
    ret_val = (char*) malloc(30000*sizeof(char));
    if (ret_val == NULL) {
        fprintf(stderr, "Out of memory when using encrypt at backup\n");
        return NULL;
    }

    if (str == NULL || strlen(str) == 0) {
        sprintf(ret_val, "NULL");
        return ret_val;
    }
    temp = (char*) malloc(10*sizeof(char));
    len = strlen(str);
    for (i = 0; i < len; i++) {
        if (i == len - 1)
            sprintf(temp, "%d", str[i]);
        else 
            sprintf(temp, "%d|", str[i]);
            
        if (i == 0)
            sprintf(ret_val, "%s", temp);
        else
            strcat(ret_val, temp);
    }
    free(temp);
    return ret_val;
}

/*
char *decrypt(char *str): decrypt the ascii string and return the pointer to original string
*/
char *decrypt(char *str) {
    char *ret_val, *token, *temp;
    int temp_int, count;

    count = 0;

    if (strcmp("NULL", str) == 0) {
        return NULL;
    }

    ret_val = (char*) malloc(10000*sizeof(char));
    temp = (char*) malloc(10*sizeof(char));
    token = strtok(str, "|");

    while (token != NULL) {
        temp_int = atoi(token);
        sprintf(temp, "%c", temp_int);
        if (count != 0)
            strcat(ret_val, temp);
        else 
            sprintf(ret_val, "%s", temp);
        count++;
        token = strtok(NULL, "|");
    }
    free(temp);
    return ret_val;
}



/*
char *serialize_block(User *cur): 
*/
char *serialize_block(User *cur) {
    BlockedUser *ptr = cur->block_head;
    char *e_username, *ret_val;
    int count = 0;

    // empty block head
    if (ptr == NULL) {
        return NULL;
    }

    // maloc space for the string
    ret_val = (char*) malloc(10000*sizeof(char));
    if (ret_val == NULL) {
        fprintf(stderr, "Out of memory when using serialize_block at backup\n");
        return NULL;
    }
    for (; ptr != NULL; ptr = ptr->next) {
        // encrypt string
        e_username = encrypt(ptr->username);

        // add space between encrypted block
        strcat(e_username, " ");

        if (count == 0) {
            sprintf(ret_val, e_username);
        } else {
            strcat(ret_val, e_username);
        }
        count++;
        free(e_username);
    }

    return ret_val;
}

/*
TODO: Change this function if have time
char *serialize_mail(User *cur)
*/
char *serialize_mail(User *cur) {
    Mail *ptr = cur->mail_head;
    char *e_username, *e_title, *e_message, *ret_val, *temp;
    int count = 0;

    // empty block head
    if (ptr == NULL) {
        return NULL;
    }

    // maloc space for the string (10 MB) might be bottle neck
    ret_val = (char*) malloc(10000*sizeof(char));
    if (ret_val == NULL) {
        fprintf(stderr, "Out of memory when using serialize_mail at backup\n");
        return NULL;
    }

    temp = (char*) malloc(10*sizeof(char));

    for (; ptr != NULL; ptr = ptr->next) {

        // encrypt strings
        e_username = encrypt(ptr->username);
        e_title = encrypt(ptr->title);
        e_message = encrypt(ptr->message);

        // add space between encrypted block
        strcat(e_username, " ");
        strcat(e_title, " ");
        strcat(e_message, " ");

        if (count == 0) {
            sprintf(ret_val, e_username);
        } else {
            strcat(ret_val, e_username);
        }

        strcat(ret_val, e_title);
        strcat(ret_val, e_message);
        sprintf(temp, "%d %li ", ptr->status, ptr->date);
        strcat(ret_val, temp);

        count++;
        free(e_username);
        free(e_title);
        free(e_message);
    }

    free(temp);
    return ret_val;
}

/*
void serialize(): save the accounts information to a file given the file name as argument
Format to save account information to file:
Users: $(name, pwd, ${info}$, win_match, loss_match, draw_match,  ${block_users}$, ${mails}$,)$, 
Block_users: name, 
mails: ^(name, status, **(title)**, **(message)**)^, 
TODO: call this function from myserver.c every 5 minutes
*/
void serialize(char *file_name) {

    if (fork() != 0)
        return; 
    
    char *e_username, *e_pwd, *e_info, *e_mail, *e_block, *raw_mail, *raw_block;
    User *cur = user_head;

    // Save acoount information to file_name file
    FILE* file = fopen(file_name, "w");
    if (file == NULL) {
        exit(1);
    }

    for (;cur != NULL; cur = cur->next) {
        // process raw info of mail
        raw_mail = serialize_mail(cur);
    
        // process raw info of block
        raw_block = serialize_block(cur);
        
        // encrypt info
        e_username = encrypt(cur->username);
        e_pwd = encrypt(cur->password);
        e_info = encrypt(cur->info);
        e_mail = encrypt(raw_mail);
        e_block = encrypt(raw_block);
        
        
        // open object and save username
        fprintf(file, "%s ", e_username);
        // save password
        fprintf(file, "%s ", e_pwd);
        // save info
        fprintf(file, "%s ", e_info);
        // save mail
        fprintf(file, "%s ",e_mail);
        // save block 
        fprintf(file, "%s ",e_block);
        // save win_match
        fprintf(file, "%d ", cur->win_match);
        // save loss_match
        fprintf(file, "%d ", cur->loss_match);
        // save draw_match
        fprintf(file, "%d ", cur->draw_match);
        // save quite status
        fprintf(file, "%d ", cur->quiet);

        /* TODO: do later
        // save block_user
        serialize_block(file, cur->block_head);
        // save mail
        serialize_mail(file, cur->mail_head);
        */
        // free encrypted string
        free(e_username);
        free(e_pwd);
        free(e_info);
        free(e_mail);
        free(e_block);

        if (raw_block != NULL) 
            free(raw_block);
        if (raw_mail != NULL)
            free(raw_mail);
    }

    fclose(file);
    exit(0);
}

/*
void deserialize_mail(char *mail, char *username): deserialize the mail given the decrypted block 
of mail and the username
*/
void deserialize_mail(char *mail, char *username) {
    char *name, *title, *message, *free_mail;
    char *d_name, *d_title, *d_message;
    int status, so_far;
    time_t date;

    free_mail = mail;

    if (mail == NULL)
        return;

    name = (char*) malloc(200*sizeof(char));
    title = (char*) malloc(200*sizeof(char));
    message = (char*) malloc(1000*sizeof(char));
    while (sscanf(mail, "%s %s %s %d %li %n", name, title, message, &status, &date, &so_far) > 0) {
        mail += so_far;
        d_name = decrypt(name);
        d_title = decrypt(title);
        d_message = decrypt(message);
        createmail_backup(username, d_name, d_title, d_message, status, date);
        free(d_name);
        free(d_title);
        free(d_message);
    }

    free(name);
    free(title);
    free(message);
    free(free_mail);
}

/*
void deserialize_block(char *block, User *user): deserialize the blocks given the decrypted block 
of block and the username
*/
void deserialize_block(char *block, User *user) {
    char *name, *d_name, *free_block;
    int so_far;

    if (block == NULL)
        return;

    free_block = block;
    name = (char*) malloc(200*sizeof(char));
    while (sscanf(block, "%s %n", name, &so_far) > 0) {
        block += so_far;
        d_name = decrypt(name);
        create_blocked_user(d_name, user);
        free(d_name);
    }
    free(name);
    free(free_block);
}


/*
void deserialize(char *file_name): It will retrieve information from the file given the file
name as argument
WARNING: this will try to override user_head, use that only when user_head is empty
*/
void deserialize(char *file_name) {
    char *name, *pwd, *info, *mail, *block;
    int win_match, loss_match, draw_match, quiet;
    // pointer to pointer of head
    User **ptr_ptr = &user_head;
    // pointer to head
    User *ptr = user_head;


    name = (char*) malloc(1200*sizeof(char));
    pwd = (char*) malloc(1200*sizeof(char));
    info = (char*) malloc(1200*sizeof(char));
    block = (char*) malloc(12000*sizeof(char));
    mail = (char*) malloc(10000*sizeof(char));

    // save account information to file_name file
    FILE* file = fopen(file_name, "r");
    if (file == NULL) {
        exit(2);
    }

    while(fscanf(file, "%s %s %s %s %s %d %d %d %d", name, pwd, info, mail, block, &win_match, &loss_match, &draw_match, &quiet) > 0) {
        printf("DEBUG: work inside scanf\n");
        // allocate memory for new user
        ptr = malloc(sizeof(User));
        if (ptr == NULL) {
            fprintf(stderr, "Out of memory when using create_user function\n");
            return;
        }

        // set it to the memory
        *ptr_ptr = ptr;

        // set username
        ptr->username = decrypt(name);
        
        // set password
        ptr->password = decrypt(pwd);

        // set info
        ptr->info = decrypt(info);

        // set win match
        ptr->win_match = win_match;
        // set loss match
        ptr->loss_match = loss_match;
        // set draw match
        ptr->draw_match = draw_match;
        // TODO set quite mode
        ptr->quiet = quiet;

        // process mail
        deserialize_mail(decrypt(mail), ptr->username);
    
        // process block
        deserialize_block(decrypt(block), ptr);

        
        ptr->next = NULL;
        ptr_ptr = &(ptr->next);
    }

    free(name);
    free(pwd);
    free(info);
    free(block);
    free(mail);

}

