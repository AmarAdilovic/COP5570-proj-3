import { Box, Button, Dialog, DialogActions, DialogContent, DialogTitle, Grid, TextField } from "@mui/material"
import { Square, SquareProps } from "./Square"
import { useState } from "react"


export const Game: React.FC<SquareProps> = ({
    onClick
}) => {
  const [command, setCommand] = useState('')
  const [openDialog, setOpenDialog] = useState(false)
  const [serverResponse, setServerResponse] = useState('')
  const rows = [1, 2, 3]
  const columns = [1, 2, 3]

  const handleCommandChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setCommand(event.target.value)
  }

  const handleClose = () => {
    setOpenDialog(false)
    setServerResponse('')
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
      return response.text()
    })
    .then(data => {
      console.log('Success:', data)
      setOpenDialog(true)
      setServerResponse(data)
    })
    .catch((error) => {
      console.error('Error:', error)
      setOpenDialog(false)
      setServerResponse('')
    })
  }

  return (
    <>
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
      <Grid container flexDirection={'column'}>
        {
          columns.map((column) => (
            <Grid item key={'column-' + column}>
              {
                rows.map((row) => (
                  <Square
                    key={'square-' + column + row}
                    onClick={onClick}
                    // color={(column % 2 === 0) ? 'black' : 'white'}
                    // side={(row % 2 === 0) ? 1 : 2}
                  />
                ))
              }
            </Grid>
          ))
        }
      </Grid>
    }
    <Dialog open={openDialog} onClose={handleClose} aria-labelledby="dialog-title">
        <DialogTitle id="dialog-title">Server Response</DialogTitle>
        <DialogContent>
            <pre>{serverResponse}</pre>
        </DialogContent>
        <DialogActions>
            <Button onClick={handleClose}>Close</Button>
        </DialogActions>
    </Dialog>
    </>
  )
}
