const internalIp = require('internal-ip');
const osc = require('osc');

// Change this to match the port you selected in the Arduino IDE
const SERIAL_DEVICE = '/dev/cu.usbmodem14201';

// Change this if you want to talk to a different OSC device
const REMOTE_ADDRESS = '127.0.0.1';
const REMOTE_PORT = 9999;

// You shouldn't need to change these
const LOCAL_ADDRESS = '0.0.0.0';
const LOCAL_PORT = 8888;
const SERIAL_BITRATE = 57600;

// Set up the network port
const inboundUdpPort = new osc.UDPPort({
  localAddress: LOCAL_ADDRESS,
  localPort: LOCAL_PORT,
  });

const outboundUdpPort = new osc.UDPPort({
  remoteAddress: REMOTE_ADDRESS,
  remotePort: REMOTE_PORT,
});

// Set up the serial port to talk to the Arduino
const serialPort = new osc.SerialPort({
  bitrate: SERIAL_BITRATE,
  devicePath: SERIAL_DEVICE,
});

let serialPortReady = false;

// Once our inbound network port is ready, print our IP address and start listening for messages
inboundUdpPort.on('ready', () => {
  console.log('LAN IP Address:', internalIp.v4.sync());
  console.log('Listening on port', inboundUdpPort.options.localPort);

  // Forward incoming messages over serial
  inboundUdpPort.on('message', (msg) => {
    console.log('Network to device:', msg);
    serialPort.send(msg);
  });
});

// Once our outbount network port is ready, indicate where we will send messages
outboundUdpPort.on('ready', () => {
  console.log(
    'Forwarding messages to',
    outboundUdpPort.options.remoteAddress,
    'port',
    outboundUdpPort.options.remotePort
  );
});

// Once we've connected to the serial port, start listening for messages
serialPort.on('ready', () => {
  console.log('Connected to serial device', serialPort.options.devicePath);
  serialPortReady = true;

  serialPort.on('message', (msg) => {
    console.log('Device to network:', msg);
    outboundUdpPort.send(msg);
  });
});

// Catch errors on all ports
inboundUdpPort.on('error', (error) => {
  console.error(error.message || error);
  console.error(`There may be another server running on port ${LOCAL_PORT}.`);
  process.exit();
});

outboundUdpPort.on('error', (error) => {
  console.error(error.message || error);
  process.exit();
});

serialPort.on('error', (error) => {
  console.log(error.message || error);

  if (!serialPortReady) {
    // Failed to open serial port, print some helpful instructions, then bail
    console.error('Ensure you are using the correct serial device and that the Arduino serial monitor is closed.');
    process.exit();
  }
});

// Start listening for messages from the network
inboundUdpPort.open();

// Enable sending messages to the network
outboundUdpPort.open();

// Connect to the serial port to relay messages to the Arduino
serialPort.open();
