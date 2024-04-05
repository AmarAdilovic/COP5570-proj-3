import { useState } from 'react'
import './App.css'

import { Game } from './components/Game'
import { NewUserForm } from './components/NewUserForm'

function App() {
  const [username, setUsername] = useState('')
  const [password, setPassword] = useState('')
  // const [userLoggedIn, setUserLoggedIn] = useState(false)


  const handleUsernameChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setUsername(event.target.value);
  }

  const handlePasswordChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setPassword(event.target.value);
  }

  // const handleClick: React.MouseEventHandler<HTMLButtonElement> = () => {
  //   setCount((count) => count + 1)
  // };


  return (
    <>
      <h1>Tic-Tac-Toe GUI</h1>
      <p>
        Click on the buttons to select a square
      </p>
      <NewUserForm
        handleUsernameChange={handleUsernameChange}
        handlePasswordChange={handlePasswordChange}
        username={username}
        password={password}
      />
      <div className="card">
        {/* <button onClick={handleClick}>
          count is {count}
        </button> */}
        <Game/>
      </div>
    </>
  )
}

export default App
