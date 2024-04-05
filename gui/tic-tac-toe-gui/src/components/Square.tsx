import { Button } from "@mui/material"
import CloseIcon from '@mui/icons-material/Close'
import RadioButtonUncheckedIcon from '@mui/icons-material/RadioButtonUnchecked'
import { useState } from "react"

// export type UserColor = ["black", "white"]
// export type UserSide = [1, 2]

export interface SquareProps {
    onClick?: React.MouseEventHandler<HTMLButtonElement>
    color?: string
    side?: number
}

export const Square: React.FC<SquareProps> = ({
    color
}) => {
  // 0 is neutral, 1 is X, 2 is 0 
  const [side, setSide] = useState(0)

  const handleClick: React.MouseEventHandler<HTMLButtonElement> = () => {
    if (side == 2) setSide(0)
    else setSide((side) => side + 1)
  }

  return (
    <Button
        onClick={handleClick}
        variant='contained'
        sx={{ height: '5rem', width: '5rem', margin: '0.25rem' }}
    >
        {
            (side == 0)
            ? <></>
            : (side === 1)
                ? <CloseIcon sx={{ height: '4rem', width: '4rem', color }}/>
                : <RadioButtonUncheckedIcon sx={{ height: '4rem', width: '4rem', color }}/>

        }        
    </Button>
  )
}
