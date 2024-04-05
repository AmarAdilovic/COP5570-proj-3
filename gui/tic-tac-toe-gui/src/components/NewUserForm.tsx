import React, { useEffect, useState } from 'react';
import Container from '@mui/material/Container'
import TextField from '@mui/material/TextField'
import Button from '@mui/material/Button'
import Box from '@mui/material/Box'
import Typography from '@mui/material/Typography'
import { Checkbox, Dialog, DialogContent, FormControlLabel } from '@mui/material'

export interface NewUserFormProps {
  handleUsernameChange: (event: React.ChangeEvent<HTMLInputElement>) => void
  handlePasswordChange: (event: React.ChangeEvent<HTMLInputElement>) => void
  username: string
  password: string
}

export const NewUserForm: React.FC<NewUserFormProps> = ({
  handleUsernameChange,
  handlePasswordChange,
  username,
  password
}) => {
  // used for the dialog window
  const [open, setOpen] = useState(true)
  const [isRegistering, setIsRegistering] = useState(false)

  
  useEffect(() => {
    console.debug("Is the user registering?: ", isRegistering)
  }, [isRegistering])

  const handleRegisteringChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setIsRegistering(event.target.checked)
  }

  // on the submission of the form
  const handleSubmit = (event: React.FormEvent) => {
    event.preventDefault()

    console.log('Submitted with username:', username, 'and password:', password);
    fetch('http://localhost:3000/api/telnet/register', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        username,
        password,
        isRegistering
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
      console.log('Success:', data)
      setOpen(false)
    })
    .catch((error) => {
      console.error('Error:', error)
    })
  };

  return (
    <Dialog open={open}>
      <DialogContent>
        <Container maxWidth="sm">
          <Box
            sx={{ marginTop: 8, display: 'flex', flexDirection: 'column', alignItems: 'center' }}
          >
            <Typography component="h1" variant="h5">
              Sign in
            </Typography>
            <Box component="form" onSubmit={handleSubmit} noValidate sx={{ mt: 1 }}>
              <TextField
                margin="normal" required fullWidth
                id="username" label="Username" name="username" autoComplete="username"
                autoFocus value={username} onChange={handleUsernameChange}
              />
              <TextField
                margin="normal" required fullWidth
                name="password" label="Password" type="password" id="password" autoComplete="current-password"
                value={password}
                onChange={handlePasswordChange}
              />
              <FormControlLabel
                control={<Checkbox checked={isRegistering} onChange={handleRegisteringChange}/>}
                label="Are you registering an account for the first time?"
              />
              <Button type="submit" fullWidth variant="contained" sx={{ mt: 3, mb: 2 }}>
                Sign In
              </Button>
            </Box>
          </Box>
        </Container>
      </DialogContent>
    </Dialog>
  )
}