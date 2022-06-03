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
    const senderGroup = msg.args[0].value;
    const senderPlayer = msg.args[1].value;
    const clientKey = `${senderGroup}/${senderPlayer}`;

    // Keep track of clients (by group name and player number) so we can send messages back to them
    clients[clientKey] = {
      group: senderGroup,
      address: request.address,
      port: request.port,
    };

    console.log('<-', `${request.address}:${request.port}`, msg.address, msg.args.map(a => a.value));

    // Broadcast the message to all known clients in the same group
    Object.entries(clients).forEach(([clientKey, client]) => {
      if (client.group == senderGroup) {
        console.log('   ->', `${client.address}:${client.port}`, msg.address, msg.args.map(a => a.value));
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
