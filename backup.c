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
char *encrypt(char *str): encrypt string into ascii and return the pointer to ascii string
*/
char *encrypt(char *str) {
    char *ret_val, *temp;
    int i, len;
    ret_val = (char*) malloc(3600*sizeof(char));
    if (str == NULL) {
        sprintf(ret_val, "NULL");
        return ret_val;
    }
    temp = (char*) malloc(10*sizeof(char));
    len = strlen(str);
    for (i = 0; i < len; i++) {
        if (i == len)
            sprintf(temp, "%d", str[i]);
        else 
            sprintf(temp, "%d|", str[i]);
        printf("temp: %s\n", temp);
        printf("ret_val: %s\n", ret_val);
        if (i == 0)
            sprintf(ret_val, temp);
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
    int temp_int;
    ret_val = (char*) malloc(1200*sizeof(char));
    if (strcmp("NULL", str) == 0) {
        sprintf(ret_val, "");
        return ret_val;
    }
    temp = (char*) malloc(10*sizeof(char));
    token = strtok(str, "|");

    while (token != NULL) {
        printf("%s\n", token);
        temp_int = atoi(token);
        sprintf(temp, "%c", temp_int);
        strcat(ret_val, temp);
        token = strtok(NULL, "|");
    }
    free(temp);
    return ret_val;
}



/*
TODO: Change this function if have time
void serialize_block(FILE *file, Blocked_user *cur): save block users to given file
${block_users}$
Block_users: name, 
*/
void serialize_block(FILE *file, Blocked_user *cur) {
    fprintf(file, "${");
    for (; cur != NULL; cur = cur->next) {
        // save blocked_user username
        fprintf(file, "%s, ", cur->username);
    }
    fprintf(file, "}$");
}

/*
TODO: Change this function if have time
void serialize_mail(FILE *file, Mail *cur): save mail to given file
${mails}$
mails: ^(name, status, **(title)**, **(message)**)^,  
*/
void serialize_mail(FILE *file, Mail *cur) {
    fprintf(file, "${");
    for (; cur != NULL; cur = cur->next) {
        // save sender username
        fprintf(file, "^(%s, ", cur->username);
        // save mail status
        fprintf(file, "%d, ", cur->status);
        // save mail title
        fprintf(file, "%s, ", cur->title);
        // save mail message
        fprintf(file, "%d)^, ", cur->message);
    }
    fprintf(file, "}$");
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
    char *e_username, *e_pwd, *e_info;
    User *cur = user_head;

    // Save acoount information to file_name file
    FILE* file = fopen(file_name, "w");
    if (file == NULL) {
        exit(1);
    }

    for (;cur != NULL; cur = cur->next) {
        // encrypt info
        e_username = encrypt(cur->username);
        e_pwd = encrypt(cur->password);
        e_info = encrypt(cur->info);
        
        
        // open object and save username
        fprintf(file, "%s ", e_username);
        // save password
        fprintf(file, "%s ", e_pwd);
        // save info
        fprintf(file, "%s ", e_info);
        // save win_match
        fprintf(file, "%d ", cur->win_match);
        // save loss_match
        fprintf(file, "%d ", cur->loss_match);
        // save draw_match
        fprintf(file, "%d ", cur->draw_match);

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
    }

    fclose(file);
}

/*
void deserialize(char *file_name): It will retrieve information from the file given the file
name as argument
WARNING: this will try to override user_head, use that only when user_head is empty
*/
void deserialize(char *file_name) {
    char *name, *pwd, *info;
    int win_match, loss_match, draw_match;
    // pointer to pointer of head
    User **ptr_ptr = &user_head;
    // pointer to head
    User *ptr = user_head;


    name = (char*) malloc(1200*sizeof(char));
    pwd = (char*) malloc(1200*sizeof(char));
    info = (char*) malloc(1200*sizeof(char));

    // save account information to file_name file
    FILE* file = fopen(file_name, "r");
    if (file == NULL) {
        exit(2);
    }

    while(fscanf(file, "%s %s %s %d %d %d", name, pwd, info, &win_match, &loss_match, &draw_match) > 0) {
        // allocate memory for new user
        ptr = malloc(sizeof(User));
        if (ptr == NULL) {
            fprintf(stderr, "Out of memory when using create_user function\n");
            return;
        }

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

        // set it to the memory
        *ptr_ptr = ptr;
        ptr->next = NULL;
        ptr_ptr = &(ptr->next);
    }

    free(name);
    free(pwd);
    free(info);

}

