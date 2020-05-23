const express = require('express');
const internalIp = require('internal-ip');
const osc = require('osc');
const WebSocket = require('ws');

// You shouldn't need to change these
const LOCAL_ADDRESS = '0.0.0.0';
const OSC_UDP_PORT = 9999;
const WEB_SERVER_PORT = 3000;

// Set up the network port
const udpPort = new osc.UDPPort({
  localAddress: LOCAL_ADDRESS,
  localPort: OSC_UDP_PORT,
});

// Create an express web server
const app = express();
const server = app.listen(WEB_SERVER_PORT, () => {
  console.log('Web server listening on port', WEB_SERVER_PORT);
});

// expose the local public folder for inluding files js, css etc..
app.use(express.static('public'));

// Create a WebSocket server
const wss = new WebSocket.Server({ server: server });

// Wait for a WebSocket connection
wss.on('connection', (socket, request) => {
  // Create an OSC port over the WebSocket
  const webSocketPort = new osc.WebSocketPort({ socket: socket });

  // When messages arrive, print them to the console and send them over the network
  webSocketPort.on('message', (msg) => {
    console.log('Web to network:', msg);
    udpPort.send(msg);
  });

  // Handle errors
  webSocketPort.on('error', (error) => {
    console.error(error);
  });

  webSocketPort.on('close', () => {
    console.log(request.socket.remoteAddress, 'disconnected');
  });

  // Store the OSC port in the socket object so we can retrieve it later
  socket.oscPort = webSocketPort;

  // Open the OSC port
  webSocketPort.open();
  console.log(request.socket.remoteAddress, 'connected');
})

// Once our network port is ready, start listening for messages and print our IP address
udpPort.on('ready', () => {
  // When messages arrive, print them to the console and send them over the WebSocket
  udpPort.on('message', (msg) => {
    console.log('Network to web:', msg);
    wss.clients.forEach((client) => {
      if (client.readyState === WebSocket.OPEN) {
        client.oscPort.send(msg);
      }
    })
  });

  console.log('LAN IP address:', internalIp.v4.sync());
  console.log('Listening for OSC messages on port', OSC_UDP_PORT);
});

// Catch errors on both ports
udpPort.on('error', (error) => {
  console.error(error);
  console.error('There may be another server running on this port.');
  process.exit();
});

// Start listening for messages from the network
udpPort.open();
