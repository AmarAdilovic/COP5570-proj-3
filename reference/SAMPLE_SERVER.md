# initial behavior

adilovic@linprog8.cs.fsu.edu:~>`telnet linprog6 56666`
Trying 128.186.120.190...
Connected to linprog6.
Escape character is '^]'.
                       -=-= AUTHORIZED USERS ONLY =-=-
You are attempting to log into online tic-tac-toe Server.
Please be advised by continuing that you agree to the terms of the
Computer Access and Usage Policy of online tic-tac-toe Server.



username (guest):

# if user enters guest

username (guest): `guest`

Commands supported:
  who                     # List all online users
  stats [name]            # Display user information
  game                    # list all current games
  observe <game_num>      # Observe a game
  unobserve               # Unobserve a game
  match <name> <b|w> [t]  # Try to start a game
  <A|B|C><1|2|3>          # Make a move in a game
  resign                  # Resign a game
  refresh                 # Refresh a game
  shout <msg>             # shout <msg> to every one online
  tell <name> <msg>       # tell user <name> message
  kibitz <msg>            # Comment on a game when observing
  ' <msg>                 # Comment on a game
  quiet                   # Quiet mode, no broadcast messages
  nonquiet                # Non-quiet mode
  block <id>              # No more communication from <id>
  unblock <id>            # Allow communication from <id>
  listmail                # List the header of the mails
  readmail <msg_num>      # Read the particular mail
  deletemail <msg_num>    # Delete the particular mail
  mail <id> <title>       # Send id a mail
  info <msg>              # change your information to <msg>
  passwd <new>            # change password
  exit                    # quit the system
  quit                    # quit the system
  help                    # print this message
  ?                       # print this message
You login as a guest. The only command that you can use is
'register username password'

<guest: 0>


## if user enters an invalid command

<guest: 0> `amar1`
You are not supposed to do this.
 You can only use 'register username password' as a guest.

### if they enter another command that doesn't work
 `another invalid command`
You are not supposed to do this.
 You can only use 'register username password' as a guest.

## if user enters the register command
`register amar1 password`
User registered.

### after registering, user has to `exit`
<guest: 1> `test`
You are not supposed to do this.
 You can only use 'register username password' as a guest.

#### they can register multiple user accounts
`register amar2 test`
User registered.
<guest: 2>

## when they `exit`
<guest: 2> `exit`
Connection closed by foreign host.

## when they successfully login
adilovic@linprog8.cs.fsu.edu:~>`telnet linprog6 56666`
Trying 128.186.120.190...
Connected to linprog6.
Escape character is '^]'.
                       -=-= AUTHORIZED USERS ONLY =-=-
You are attempting to log into online tic-tac-toe Server.
Please be advised by continuing that you agree to the terms of the
Computer Access and Usage Policy of online tic-tac-toe Server.



username (guest): `amar2`
password: `password`
            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            %                                         %
             %              Welcome to               %
              %     Online Tic-tac-toe  Server      %
             %                                        %
            %                                          %
            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



Commands supported:
  who                     # List all online users
  stats [name]            # Display user information
  game                    # list all current games
  observe <game_num>      # Observe a game
  unobserve               # Unobserve a game
  match <name> <b|w> [t]  # Try to start a game
  <A|B|C><1|2|3>          # Make a move in a game
  resign                  # Resign a game
  refresh                 # Refresh a game
  shout <msg>             # shout <msg> to every one online
  tell <name> <msg>       # tell user <name> message
  kibitz <msg>            # Comment on a game when observing
  ' <msg>                 # Comment on a game
  quiet                   # Quiet mode, no broadcast messages
  nonquiet                # Non-quiet mode
  block <id>              # No more communication from <id>
  unblock <id>            # Allow communication from <id>
  listmail                # List the header of the mails
  readmail <msg_num>      # Read the particular mail
  deletemail <msg_num>    # Delete the particular mail
  mail <id> <title>       # Send id a mail
  info <msg>              # change your information to <msg>
  passwd <new>            # change password
  exit                    # quit the system
  quit                    # quit the system
  help                    # print this message
  ?                       # print this message
You have no unread message.
<amar2: 0>


### when sending a mail to a user
<amar: 1> mail puffvu hello world
Please input mail body, finishing with '.' at the beginning of a line

another test
a second test
.third test
Message sent

#### the "third test" text will not be visible to the user