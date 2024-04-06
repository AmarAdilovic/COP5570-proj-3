# COP5570-proj-3
 Online tic-tac-toe multi-plex server

## To run:

`make`

`./myserver PORT_NUM` where PORT_NUM can be any valid port (for example, 53127)

`telnet IP_ADDR PORT_NUM` from another terminal, where IP_ADDR is given by the server and PORT_NUM is set

ctrl + c to kill server

`make clean` to clean files

## To debug:

`gdb ./myserver`

then `r PORT_NUM`

## To run the GUI for the first time:

### First run the back-end
    - `cd gui/tic-tac-toe-backend`
    - `npm install` 
    - `node server.js TEL_NET_PORT_NUMBER`

### Then run the front-end
    - `cd gui/tic-tac-toe-gui`
    - `npm install`
    - `npm run dev -- --port PORT_NUMBER`
