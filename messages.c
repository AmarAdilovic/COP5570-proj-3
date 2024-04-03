/*
File: messages.c
Purpose: contains all the pre-defined messages we display to the user from the server
*/

#include "myserver.h"

// this returns the authorization warning displayed on the initial connection and the username prompt
char *initial_messsage() {
    return (
	"                       -=-= AUTHORIZED USERS ONLY =-=-\n"
	"You are attempting to log into online tic-tac-toe Server.\n"
	"Please be advised by continuing that you agree to the terms of the\n"
	"Computer Access and Usage Policy of online tic-tac-toe Server.\n"
	"\n\n\n"
	"username (guest): "
    );
}

// displays the message for when a user successfully logs in
char *welcome_message() {
    return (
	"\t\t%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n"
    "\t\t%                                         %\n"
    "\t\t %              Welcome to               %\n"
    "\t\t  %     Online Tic-tac-toe  Server      %\n"
    "\t\t %                                        %\n"
    "\t\t%                                          %\n"
    "\t\t%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n\n\n"
	);
}

// if the user enters an invalid username and password combination, we will display this message
char *login_failed_message() {
    return (
	"Login failed!!\n"
	);
}

// when the connection is closed
char *connection_closed_message() {
    return (
	"Thank you for using Online Tic-tac-toe Server.\n"
	"See you next time.\n\n"
	);
}

// when the user successfully logs in as a guest
char *guest_user_message() {
    return (
	"You login as a guest. The only command that you can use is\n"
	"'register username password'\n\n"
	);
}

// when a guest user enters a command that isn't the register command (they can also exit/quit)
char *guest_user_warning_message() {
    return (
	"You are not supposed to do this.\n"
	" You can only use 'register username password' as a guest.\n"
	);
}


// <amar: 0> exit
// You are not in the room.
// <amar: 1> Thank you for using Online Tic-tac-toe Server.
// See you next time.

// when logging out
// <amar: 3> You are not in the room.
