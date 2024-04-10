import { Box, Button, Dialog, DialogActions, DialogContent, DialogTitle, Grid, TextField, Typography } from "@mui/material"
import { useState } from "react"
import { GameBoard } from "./GameBoard"

export interface GameBoardProps {
}

export const getRow = (row: number) => {
  switch (row) {
    case 0:
      return 'A'
    case 1:
      return 'B'
    case 2:
      return 'C'
    default:
      return ''
  }
}

export const Game: React.FC<GameBoardProps> = () => {
  const [command, setCommand] = useState('')
  const [openDialog, setOpenDialog] = useState(false)
  const [serverResponse, setServerResponse] = useState('')

  const [boardInitialized, setBoardInitialized] = useState(false)
  const [boardState, setBoardState] = useState([''])
  const [blackPlayerName, setBlackPlayerName] = useState('')
  const [blackPlayerTime, setBlackPlayerTime] = useState(-1)
  const [whitePlayerName, setWhitePlayerName] = useState('')
  const [whitePlayerTime, setWhitePlayerTime] = useState(-1)

  const handleCommandChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setCommand(event.target.value)
  }

  const handleClose = () => {
    setOpenDialog(false)
    setServerResponse('')
  }

  function isMoveFormat(inputString: string) {
    const regex = /^[ABC][123]$/;
    return regex.test(inputString)
  }
  
  function parseGameData(inputString: string) {
    // Initialize an object to hold the parsed data
    const gameData = {
      blackPlayer: '',
      whitePlayer: '',
      blackTime: 0,
      whiteTime: 0,
      board: ['']
    };
  
    // Extract players and times using regular expressions
    const playerRegex = /Black:\s*(\w+)\s*White:\s*(\w+)/;
    const timeRegex = /Time:\s*(\d+)\s*Time:\s*(\d+)/;
  
    const playerMatches = playerRegex.exec(inputString);
    const timeMatches = timeRegex.exec(inputString);
  
    if (playerMatches) {
      gameData.blackPlayer = playerMatches[1];
      gameData.whitePlayer = playerMatches[2];
    }
  
    if (timeMatches) {
      gameData.blackTime = parseInt(timeMatches[1], 10);
      gameData.whiteTime = parseInt(timeMatches[2], 10);
    }
  
    const boardRegex = /^[ABC]\s+[O.#]+\s+[O.#]+\s+[O.#]+$/gm;
    const matches = inputString.match(boardRegex);
  
    if (matches) {
      matches.forEach((match) => {
        const row = match.trim().split(/\s+/).slice(1); // Ignore the row label (A, B, C)
        row.forEach(element => {
            gameData.board.push(element)
          })
      })
    }
    gameData.board.shift()
  
    return gameData;
  }
  
  
  
  // on the submission of the form
  const handleSubmit = (event: React.FormEvent) => {
    event.preventDefault()

    fetch('http://localhost:3000/api/telnet/command', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        command
      }),
    })
    .then(response => {
      // Checks if the status code is not in the range 200-299
      if (!response.ok) {
          throw new Error('Network response was not ok')
      }
      return response.json()
    })
    .then(data => {
        console.log('Success:', data.message)
        setOpenDialog(true)
        setServerResponse(data.message)
        if (isMoveFormat(command) || command === 'refresh') {
            // parse the message to check if it is an invalid move (i.e., not the user's turn?)
            const parsedGame = parseGameData(data.message)
            console.log(parsedGame)

            // the command might be in the move format but might be an invalid move
            if (parsedGame.board.length === 9) {
              setBoardState(parsedGame.board)
              if (!boardInitialized) setBoardInitialized(true)
              setBlackPlayerName(parsedGame.blackPlayer)
              setBlackPlayerTime(parsedGame.blackTime)
              setWhitePlayerName(parsedGame.whitePlayer)
              setWhitePlayerTime(parsedGame.whiteTime)
            }
        }

        if (command === 'quit' || command === 'exit') {
          fetch('http://localhost:3000/api/telnet/quit', {
            method: 'POST',
            headers: {
              'Content-Type': 'application/json',
            },
          })
          .then(response => {
            // Checks if the status code is not in the range 200-299
            if (!response.ok) {
                throw new Error('Network response was not ok')
            }
            return response.json()
          })
          .then(async data => {
              console.log('Success:', data.message)
              setOpenDialog(true)
              setServerResponse('Successfully quit from the server, reloading page in 3 seconds')
              await new Promise(resolve => setTimeout(resolve, 3000))
              window.location.reload()
          })
          .catch((error) => {
            console.error('Error:', error)
            setOpenDialog(false)
            setServerResponse('')
          })
      
        }

    })
    .catch((error) => {
      console.error('Error:', error)
      setOpenDialog(false)
      setServerResponse('')
    })
  }

  const handleMove = (move: string) => {
    fetch('http://localhost:3000/api/telnet/command', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        "command": move
      }),
    })
    .then(response => {
      // Checks if the status code is not in the range 200-299
      if (!response.ok) {
          throw new Error('Network response was not ok')
      }
      return response.json()
    })
    .then(data => {
        console.log('Move success:', data.message)
        setOpenDialog(true)
        setServerResponse(data.message)

        if (isMoveFormat(move)){
            // parse the message to check if it is an invalid move (i.e., not the user's turn?)
            const parsedGame = parseGameData(data.message)
            console.log(parsedGame)

            // the command might be in the move format but might be an invalid move
            if (parsedGame.board.length === 9) {
              setBoardState(parsedGame.board)
              setBlackPlayerName(parsedGame.blackPlayer)
              setBlackPlayerTime(parsedGame.blackTime)
              setWhitePlayerName(parsedGame.whitePlayer)
              setWhitePlayerTime(parsedGame.whiteTime)
            }
        }
    })
    .catch((error) => {
      console.error('Move error:', error)
      setOpenDialog(false)
      setServerResponse('')
    })
  }

  const handleSquareClick = (index: number) => {
    // This is for the 3x3 board
    // columns are 1, 2, 3
    const column = (index % 3) + 1

    // rows are A, B, C
    const row = Math.floor(index / 3)
    const rowLetter = getRow(row)

    const move = rowLetter + column
    console.log(`Button clicked at row ${row}, column ${column}. Move interpreted as ${move}`)
    handleMove(move)
  }

  return (
    <>
    <Typography marginTop={'5rem'}> Submit a command you would like to run on the telnet server. Enter "help" or "?" for more assistance. </Typography>
    <Box component="form" onSubmit={handleSubmit} noValidate sx={{ mt: 1 }}>
        <TextField
        margin="normal" required fullWidth
        id="command" label="Enter a command" name="command" autoComplete="command"
        autoFocus value={command} onChange={handleCommandChange}
        />
        <Button type="submit" fullWidth variant="contained" sx={{ mt: 3, mb: 2 }}>
            Submit Command
        </Button>
    </Box>
    {
      (boardInitialized)
      ? (
          <>
            <Grid container marginTop={'3rem'} flexDirection={'column'}>
              <Grid item display={'flex'}>
                <Typography > {'Black: ' + blackPlayerName} </Typography>
                <Typography marginLeft={'auto'}> {'White: ' + whitePlayerName} </Typography>
              </Grid>
              <Grid item display={'flex'}>
                <Typography > {'Time: ' + blackPlayerTime} </Typography>
                <Typography marginLeft={'auto'}> {'Time: ' + whitePlayerTime} </Typography>
              </Grid>
            </Grid>
            <GameBoard boardState={boardState} handleClick={handleSquareClick}/>
          </>
        )
      : <></>
    }
    <Dialog open={openDialog} onClose={handleClose} aria-labelledby="dialog-title">
        <DialogTitle id="dialog-title">Server Response</DialogTitle>
        <DialogContent>
            <pre style={{whiteSpace: 'pre-wrap', wordWrap: 'break-word'}}>{serverResponse}</pre>
        </DialogContent>
        <DialogActions>
            <Button onClick={handleClose}>Close</Button>
        </DialogActions>
    </Dialog>
    </>
  )
}
