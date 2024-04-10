import { useState } from 'react'
import './App.css'

import { Game } from './components/Game'
import { NewUserForm } from './components/NewUserForm'
import { Typography } from '@mui/material'

function App() {
  const [username, setUsername] = useState('')
  const [password, setPassword] = useState('')

  const handleUsernameChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setUsername(event.target.value);
  }

  const handlePasswordChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setPassword(event.target.value);
  }

  return (
    <>
      <h1>Tic-Tac-Toe GUI</h1>
      <Typography>
        {'Logged in as: ' + username}
      </Typography>

      <NewUserForm
        handleUsernameChange={handleUsernameChange}
        handlePasswordChange={handlePasswordChange}
        username={username}
        password={password}
      />
      <div className="card">
        <Game/>
      </div>
    </>
  )
}

export default App
