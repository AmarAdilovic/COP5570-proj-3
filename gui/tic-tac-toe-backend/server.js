const express = require('express');
const { Telnet } = require('telnet-client');

const app = express();
const port = 3000;
const telnet_port_number = process.argv[2];

app.use(express.json()); // Middleware to parse JSON bodies

// Allow CORS for development purposes
app.use((req, res, next) => {
  res.header('Access-Control-Allow-Origin', '*'); // Adjust in production
  res.header(
    'Access-Control-Allow-Headers',
    'Origin, X-Requested-With, Content-Type, Accept'
  );
  next();
});

// Example route to send a command to the telnet server and receive a response
app.post('/api/telnet/register', async (req, res) => {
  console.log("Register request body: ", req.body);
  let connection = new Telnet();
  const params = {
    host: '0.0.0.0',
    port: telnet_port_number,
    negotiationMandatory: false,
    timeout: 15000,
  };

  try {
    connection.connect(params)
    .then(prompt => {
        console.log('Prompt from the connection.connect: ', prompt)
        // connection.exec(cmd)
        // .then(res => {
        //   console.log('promises result:', res)
        // })
      }, error => {
        console.log('promises reject:', error)
      })
      .catch(error => {
        console.error('connection.connect error:', error)
        // handle the throw (timeout)
      })
    res.json({ message: 'Success' })
  } catch (error) {
    console.error('Telnet error:', error)
    res.status(500).json({ message: 'Error communicating with telnet server', error })
  }
});

// app.post('/api/telnet/register', async (req, res) => {
//     console.log("Register request body: ", req.body);
//     let connection = new Telnet();
//     const params = {
//       host: '0.0.0.0',
//       port: telnet_port_number,
//       negotiationMandatory: false,
//       timeout: 15000,
//     };
  
//     try {
//       connection.connect(params)
//       res.json({ message: 'Success' })
//     } catch (error) {
//       console.error('Telnet error:', error)
//       res.status(500).json({ message: 'Error communicating with telnet server', error })
//     }
// });

// await connection.connect(params);
// // Assuming the command is sent in the request body
// const commandResponse = await connection.send(req.body.command);
// await connection.end();
// res.json({ message: 'Success', response: commandResponse });

app.listen(port, () => {
  console.log(`Server listening at http://localhost:${port}`);
});
