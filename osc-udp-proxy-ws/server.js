const internalIp = require('internal-ip');
const osc = require('osc');

// Change this to match the server you're trying to talk to
const SERVER_URL = 'http://127.0.0.1:3000';

// You shouldn't need to change these
const LOCAL_ADDRESS = '0.0.0.0';
const OSC_UDP_PORT = 9000;

// Connect to the server via WebSocket
const webSocketPort = new osc.WebSocketPort({ url: SERVER_URL });

// Set up the network port
const udpPort = new osc.UDPPort({
  localAddress: LOCAL_ADDRESS,
  localPort: OSC_UDP_PORT,
});

// Start listening for messages from the server and forwarding them to the network port
webSocketPort.on('message', (msg) => {
  console.log('Web to network:', msg);
  udpPort.send(msg);
});

// Once our network port is ready, start listening for messages and print our IP address
udpPort.on('ready', () => {
  // When messages arrive, print them to the console and send them over the WebSocket
  udpPort.on('message', (msg) => {
    console.log('Network to web:', msg);
    webSocketPort.send(msg);
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

webSocketPort.on('error', (error) => {
  console.error(error.message || error);
  process.exit();
});

// Get everything going
udpPort.open();
webSocketPort.open();
