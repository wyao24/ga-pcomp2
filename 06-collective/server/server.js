const ip = require('ip');
const osc = require('osc');

const LOCAL_ADDRESS = '0.0.0.0';
const OSC_UDP_PORT = process.env.PORT || 9000;

const server = new osc.UDPPort({
  localAddress: LOCAL_ADDRESS,
  localPort: OSC_UDP_PORT,
  metadata: true,
});

const clients = {};

server.on('ready', () => {
  console.log('Welcome to the collective');
  console.log('IP Address:', ip.address());
  console.log('Port:', OSC_UDP_PORT);
  console.log();

  server.on('message', (msg, _, request) => {
    const senderAddress = `${request.address}:${request.port}`;
    const senderGroup = msg.args[0].value;

    // Keep a UDPPort object for sending messages back to each the clients we've heard from
    if (clients[senderAddress] == null) {
      const newClient = {
        group: senderGroup,
        port: new osc.UDPPort({
          localAddress: LOCAL_ADDRESS,
          localPort: 0, // Let the OS pick a random, unused port
          remoteAddress: request.address,
          remotePort: request.port,
          metadata: true,
        })
      };

      clients[senderAddress] = newClient;

      // Wait for the port to be open before using it
      newClient.port.on('open', () => {
        newClient.port.isOpen = true;
      });

      // If there's ever an error on the port or it gets closed, remove it from the list.
      newClient.port.on('error', (error) => {
        console.log('xx', senderAddress, 'ERROR', error);
        delete clients[senderAddress];
      });

      newClient.port.on('close', () => {
        console.log('xx', senderAddress, 'CLOSED');
        delete clients[senderAddress];
      });

      newClient.port.open();
    }

    console.log('<-', senderAddress, msg.address, msg.args.map(a => a.value));

    // Broadcast the message to all known clients in the same group
    Object.entries(clients).forEach(([clientAddress, client]) => {
      if (client.port.isOpen && client.group == senderGroup) {
        console.log('   ->', clientAddress, msg.address, msg.args.map(a => a.value));
        client.port.send(msg);
      }
    });
  });
});

server.open();
