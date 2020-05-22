const express = require('express');
const internalIp = require('internal-ip');
const osc = require('osc');

// You shouldn't need to change these
const LOCAL_ADDRESS = internalIp.v4.sync() || '0.0.0.0';
const OSC_UDP_PORT = 9999;
const WEB_SERVER_PORT = 3000;

// Set up the network port
const udpPort = new osc.UDPPort({
  localAddress: LOCAL_ADDRESS,
  localPort: OSC_UDP_PORT,
  metadata: true,
});

// Our list of open WebSockets
let openWebSockets = [];

// Create an express web server
const app = express();
const server = app.listen(WEB_SERVER_PORT, () => {
  console.log('Web server listening on port', WEB_SERVER_PORT);
});

// expose the local public folder for inluding files js, css etc..
app.use(express.static('public'));

// Create a WebSocket server
const io = require('socket.io')(server);

// Wait for a WebSocket connection
io.on('connection', (socket) => {
  // Create an OSC port over the WebSocket
  const webSocketPort = new osc.WebSocketPort({
    socket: socket,
    metadata: true,
  });

  // When messages arrive, print them to the console and send them over the network
  webSocketPort.on('message', (msg) => {
    console.log('Web to network:', msg);
    udpPort.send(msg);
  });

  // Handle errors
  webSocketPort.on('error', (error) => {
    console.error(error.message || error);
    process.exit();
  });

  // When the socket gets disconnected, remove it from the list so we stop sending to it
  socket.on('disconnect', () => {
    console.log('WebSocket disconnection from', socket.conn.remoteAddress);
    openWebSockets = openWebSockets.filter((openSocket) => openSocket !== socket);
  })

  // Add the socket to our list of sockets to transmit to
  openWebSockets.push(socket);

  // Open the OSC port
  webSocketPort.open();

  console.log('WebSocket connection from', socket.conn.remoteAddress);
})

// Once our network port is ready, start listening for messages and print our IP address
udpPort.on('ready', () => {
  // When messages arrive, print them to the console and send them over the WebSocket
  udpPort.on('message', (msg) => {
    console.log('Network to web:', msg);
    openWebSockets.forEach((socket) => {
      socket.send(msg);
    })
  });

  console.log('Listening for OSC messages on', LOCAL_ADDRESS, 'port', OSC_UDP_PORT);
});

// Catch errors on both ports
udpPort.on('error', (error) => {
  console.error(error.message || error);
  console.error('There may be another server running on this port.');
  process.exit();
});

// Start listening for messages from the network
udpPort.open();
