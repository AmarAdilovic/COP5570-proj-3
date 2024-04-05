import { Grid } from "@mui/material"
import { Square, SquareProps } from "./Square"


export const Game: React.FC<SquareProps> = ({
    onClick
}) => {
  const rows = [1, 2, 3]
  const columns = [1, 2, 3]


  return (
    <>
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
    </>
  )
}
