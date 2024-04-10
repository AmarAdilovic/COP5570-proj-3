import { Button } from "@mui/material"
import CloseIcon from '@mui/icons-material/Close'
import RadioButtonUncheckedIcon from '@mui/icons-material/RadioButtonUnchecked'

export interface SquareProps {
    onClick: React.MouseEventHandler<HTMLButtonElement>
    side?: string
}

export const Square: React.FC<SquareProps> = ({
    onClick,
    side
}) => {

  return (
    <Button
        onClick={onClick}
        variant='contained'
        sx={{ height: '5rem', width: '5rem', margin: '0.25rem' }}
    >
        {
            (side === "#")
            ? <CloseIcon sx={{ height: '4rem', width: '4rem', color: 'black' }}/>
            : (side === "O")
                ? <RadioButtonUncheckedIcon sx={{ height: '4rem', width: '4rem', color: 'white' }}/>
                : <></>
        }        
    </Button>
  )
}
