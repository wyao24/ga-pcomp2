const express = require('express');
const osc = require('osc');
const WebSocket = require('ws');

const WEB_SERVER_PORT = process.env.PORT || 3000;

// Create an express web server
const app = express();
const server = app.listen(WEB_SERVER_PORT, () => {
  console.log('Web server listening on port', WEB_SERVER_PORT);
});

// Create a WebSocket server
const wss = new WebSocket.Server({ server: server });

// Handle new WebSocket connections
wss.on('connection', (socket, request) => {
  const remoteAddress = getRemoteAddress(request);

  // Print errors
  socket.on('error', (error) => {
    console.error(error);
  });

  // Broadcast any received messages to all of the clients
  socket.on('message', (msg) => {
    broadcast(msg);
  });

  socket.on('close', () => {
    console.log(remoteAddress, 'left');
    broadcast(osc.writeMessage({
      address: "/collective/leave",
      args: [{ type: 's', value: remoteAddress }],
    }));
  });

  console.log(remoteAddress, 'joined');

  // Say hi to the new client
  socket.send(osc.writeMessage({ address: "/collective/hi", args: [] }));

  // Tell everyone else we have a new client
  broadcast(osc.writeMessage({
    address: "/collective/join",
    args: [{ type: 's', value: remoteAddress }],
  }));

});

function broadcast(msg) {
  wss.clients.forEach((client) => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(msg);
    }
  });
}

function getRemoteAddress(request) {
  // First look for an x-forwarded-for header in case there's a proxy (like on heroku)
  const forwardedFor = request.headers['x-forwarded-for'];

  if (forwardedFor) {
    const addresses = forwardedFor.split(',');
    return addresses[addresses.length - 1];
  }
  return request.socket.remoteAddress;
}
