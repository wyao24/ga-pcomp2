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

    // Keep track of clients so we can send messages back to each the clients we've heard from
    if (clients[senderAddress] == null) {
      clients[senderAddress] = {
        group: senderGroup,
        address: request.address,
        port: request.port,
      };
    }

    console.log('<-', senderAddress, msg.address, msg.args.map(a => a.value));

    // Broadcast the message to all known clients in the same group
    Object.entries(clients).forEach(([clientAddress, client]) => {
      if (client.group == senderGroup) {
        console.log('   ->', clientAddress, msg.address, msg.args.map(a => a.value));
        server.send(msg, client.address, client.port);
      }
    });
  });

  server.on('error', (err) => {
    // TODO: can we remove clients if we get an error?
    console.error(err);
  });
});

server.open();
