import { Box, Grid, Typography } from "@mui/material"

import { Square } from "./Square"
import { getRow } from "./Game"

export interface GameBoardProps {
    boardState: string[]
    handleClick: (index: number) => void
}

export const GameBoard: React.FC<GameBoardProps> = ({
  boardState,
  handleClick
}) => {

  return (
    <Grid container flexDirection={'column'} marginTop={'2rem'} alignItems={'center'}>
      {
        Array.from({ length: 3 }, (_, rowIndex) => ( // Creates an array [0, 1, 2] for rows
          <Box display={'flex'} flexDirection={'row'} alignItems={'center'} justifyContent={'center'} key={'row-' + rowIndex}>
            <Typography fontWeight={'bold'} marginRight={'0.5rem'} fontSize={'1.5rem'}> {getRow(rowIndex)} </Typography>
            <Box display={'flex'}>
              {
                Array.from({ length: 3 }, (_, columnIndex) => { // Creates an array [0, 1, 2] for columns
                  const index = rowIndex * 3 + columnIndex; // Calculate the index in the boardState
                  return (
                  (index === 0 || index === 1 || index === 2)
                  ? (
                    <Box display={'flex'} flexDirection={'column'} alignItems={'center'} justifyContent={'center'} key={'square-grid-' + index}>
                      <Typography fontWeight={'bold'} marginRight={'0.5rem'} fontSize={'1.5rem'}> {index + 1} </Typography>
                      <Square
                        key={'square-' + index}
                        onClick={() => handleClick(index)}
                        side={boardState[index]}
                      />
                    </Box>
                    )
                  : (
                    <Square
                      key={'square-' + index}
                      onClick={() => handleClick(index)}
                      side={boardState[index]}
                    />
                    )
                  )
                })
              }
            </Box>
          </Box>
        ))
      }
    </Grid>
  )
  
}
