/*
File: mycommands.c
Purpose: contains all commands a user can instruct the server to call, 
also called by the server in certain senarios (such as on user login)
*/

#include "myserver.h"

// this returns the help menu which lists the commands available to the user
char *help_command() {
    // TODO: the pound symbol "#" infront of each command is for the commands we do not have implemented,
    // remove them as we implement the commands
    char* RETURNED_STRING = 
    "Commands supported:\n"
    "  #who                     # List all online users\n"
    "  #stats [name]            # Display user information\n"
    "  #game                    # list all current games\n"
    "  #observe <game_num>      # Observe a game\n"
    "  #unobserve               # Unobserve a game\n"
    "  #match <name> <b|w> [t]  # Try to start a game\n"
    "  #<A|B|C><1|2|3>          # Make a move in a game\n"
    "  #resign                  # Resign a game\n"
    "  #refresh                 # Refresh a game\n"
    "  #shout <msg>             # shout <msg> to every one online\n"
    "  #tell <name> <msg>       # tell user <name> message\n"
    "  #kibitz <msg>            # Comment on a game when observing\n"
    "  #' <msg>                 # Comment on a game\n"
    "  #quiet                   # Quiet mode, no broadcast messages\n"
    "  #nonquiet                # Non-quiet mode\n"
    "  #block <id>              # No more communication from <id>\n"
    "  #unblock <id>            # Allow communication from <id>\n"
    "  #listmail                # List the header of the mails\n"
    "  #readmail <msg_num>      # Read the particular mail\n"
    "  #deletemail <msg_num>    # Delete the particular mail\n"
    "  #mail <id> <title>       # Send id a mail\n"
    "  #info <msg>              # change your information to <msg>\n"
    "  #passwd <new>            # change password\n"
    "  exit                    # quit the system\n"
    "  quit                    # quit the system\n"
    "  help                    # print this message\n"
    "  ?                       # print this message\n";

    return RETURNED_STRING;
}

