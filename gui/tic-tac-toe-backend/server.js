const express = require('express');
const { Telnet } = require('telnet-client');

const app = express();
const port = 3000;
const telnet_port_number = process.argv[2];

app.use(express.json()); // Middleware to parse JSON bodies

// Allow CORS for development purposes
app.use((req, res, next) => {
  res.header('Access-Control-Allow-Origin', '*');
  res.header(
    'Access-Control-Allow-Headers',
    'Origin, X-Requested-With, Content-Type, Accept'
  );
  next();
});

// sends a command to the telnet server and receive a response
// app.post('/api/telnet/register', async (req, res) => {
//   console.log("Register request body: ", req.body);
//   let connection = new Telnet();
//   const params = {
//     host: '0.0.0.0',
//     port: telnet_port_number,
//     negotiationMandatory: false,
//     timeout: 15000,
//   };

//   try {
//     connection.connect(params)
//     .then(prompt => {
//         console.log('Prompt from the connection.connect: ', prompt)
//         // connection.exec(cmd)
//         // .then(res => {
//         //   console.log('promises result:', res)
//         // })
//       }, error => {
//         console.log('promises reject:', error)
//       })
//       .catch(error => {
//         console.error('connection.connect error:', error)
//         // handle the throw (timeout)
//       })
//     res.json({ message: 'Success' })
//   } catch (error) {
//     console.error('Telnet error:', error)
//     res.status(500).json({ message: 'Error communicating with telnet server', error })
//   }
// });

app.post('/api/telnet/register', async (req, res) => {
    console.log("Register request body: ", req.body)

    // to determine which errors should be thrown to the client
    // let loginFailed = false
    // let registerFailed = false
    //  1 === login successful, 2 === registration successful
    let statusCode = 0

    let connection = new Telnet()
    const params = {
      host: '0.0.0.0',
      port: telnet_port_number,
      setTimeout: 0,
      negotiationMandatory: false
    }

    function delay(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }

    try {
        // connection logic wrapped in a Promise
        await new Promise((resolve, reject) => {
            connection.on('ready', async function() {
                console.log('Server "ready"\n')
            });
        
            connection.on('timeout', function() {
                console.log('socket timeout!')
                // connection.end()
                // reject(new Error('Telnet connection timeout'))
            })
        
            connection.on('close', function() {
                console.log('connection closed')
                // if (loginFailed) {
                //     reject(new Error("The passed in username and password failed to login."))
                // }
                // if (registerFailed) {
                //     reject(new Error("Could not register with the given username and password."))
                // }
            })
        
            connection.on('data', async function(data) {
                const dataString = data.toString()
                console.log('Received data from the server:\n', dataString)
                if (req.body.isRegistering === true) {
                    if (dataString.includes("username (guest):")) {
                        console.log('\nUsername request received from the server!\n')
                        const guestString = 'guest'
                        console.log('Sending guest to the telnet client: ', guestString)
                        await delay(1000) // Wait for 1 second before sending
                        connection.write(guestString + "\n")
                    }
                    else if (dataString.includes("register username password")) {
                        console.log('\nRegister request received from the server!\n')
                        registerString = 'register ' + req.body.username + ' ' + req.body.password
                        console.log('Sending register command to the telnet client: ', registerString)
                        await delay(1000) // Wait for 1 second before sending
                        connection.write(registerString + "\n")
                    }
                    else if (dataString.includes("User registered.")) {
                        console.log('\nUser registered successfully, resolving\n')

                        statusCode = 2
                        resolve()
                        // connection.end()
                    } else if (dataString.includes("Please change the username")) {
                        console.log('\nRegistration failed, ending the connection\n')

                        // registerFailed = true
                        reject(new Error("Could not register with the given username and password."))
                    }
                }
                else if (req.body.isRegistering === false) {
                    if (dataString.includes("username (guest):")) {
                        console.log('\nUsername request received from the server!\n')
                        console.log('Sending username to the telnet client: ', req.body.username)
                        await delay(1000) // Wait for 1 second before sending
                        connection.write(req.body.username + "\n")
                    }
                    else if (dataString.includes("password:")) {
                        console.log('\nPassword request received from the server!\n')
                        console.log('Sending password to the telnet client: ', req.body.password)
                        await delay(1000) // Wait for 1 second before sending
                        connection.write(req.body.password + "\n")
                    }
                    else if (dataString.includes("Login failed!!") || dataString.includes("Thank you for using Online Tic-tac-toe Server")) {
                        console.log('\nLogin failed, ending the connection\n')

                        // loginFailed = true
                        // connection.end()
                        reject(new Error("The passed in username and password failed to login."))
                    } else {
                        // You can only use 'register username password' as a guest
                        console.log('\nLogin persumed success, resolving\n')

                        statusCode = 1
                        resolve()
                    }
                }
            })
            
            connection.on('failedlogin', function(prompt) {
                console.error('Failed login:', prompt)
            })
            
            connection.on('connect', function() {
                console.log('Connected to the telnet server.')
            })
    
            connection.connect(params).catch((error) => {
                // Propagates connection error
                reject(error)
            })
        })
  
        // If the promise resolves, send a successful response
        if (statusCode === 1) {
            res.json({ message: 'Successfully logged into the telnet server' })
        }
        else if (statusCode === 2) {
            res.json({ message: 'Successfully registered an account for the telnet server' })
            connection.end()
        }
        else {
            res.json({ message: 'Unknown success' })
        }

      } catch (error) {
        console.error('Telnet error:', error);
        // Communicate back the error through the HTTP response
        res.status(500).json({ message: 'Error communicating with telnet server', error: error.message });
      }
})


// await connection.connect(params);
// // Assuming the command is sent in the request body
// const commandResponse = await connection.send(req.body.command);
// await connection.end();
// res.json({ message: 'Success', response: commandResponse });

app.listen(port, () => {
  console.log(`Server listening at http://localhost:${port}`);
});
