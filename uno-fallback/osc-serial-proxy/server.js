const ip = require('ip');
const osc = require('osc');

// Change this to match the port you selected in the Arduino IDE
const SERIAL_DEVICE = '/dev/cu.usbmodem14201';

// Change these to match your receiving OSC device
const OUTBOUND_ADDRESS = '192.168.1.100';
const OUTBOUND_PORT = 9000;

// You shouldn't need to change these
const INBOUND_ADDRESS = '0.0.0.0';
const INBOUND_PORT = 8888;
const SERIAL_BITRATE = 57600;

const udpPort = new osc.UDPPort({
  localAddress: INBOUND_ADDRESS,
  localPort: INBOUND_PORT,
  remoteAddress: OUTBOUND_ADDRESS,
  remotePort: OUTBOUND_PORT,
  metadata: true,
});

// Set up the serial port to talk to the Arduino
const serialPort = new osc.SerialPort({
  bitrate: SERIAL_BITRATE,
  devicePath: SERIAL_DEVICE,
  metadata: true,
});

let serialPortReady = false;

// Once our inbound network port is ready, print our IP address and start listening for messages
udpPort.on('ready', () => {
  console.log('IP Address:', ip.address());
  console.log('Port:', udpPort.options.localPort);
  console.log();

  // Forward incoming messages over serial
  udpPort.on('message', (msg) => {
    console.log('Network to device:', msg);
    serialPort.send(msg);
  });

  // Indicate where we will send outbound messages
  console.log(
    'Forwarding messages to',
    udpPort.options.remoteAddress,
    'port',
    udpPort.options.remotePort
  );
});

// Once we've connected to the serial port, start listening for messages
serialPort.on('ready', () => {
  console.log('Connected to serial device', serialPort.options.devicePath);
  console.log();
  serialPortReady = true;

  serialPort.on('message', (msg) => {
    console.log('Device to network:', msg);
    udpPort.send(msg);
  });
});

// Catch errors on both ports
udpPort.on('error', (error) => {
  console.error(error.message || error);

  if (error.code === 'EADDRINUSE') {
    console.error(`There may be another server running on port ${INBOUND_PORT}.`);
  }
});

serialPort.on('error', (error) => {
  console.log(error.message || error);

  if (!serialPortReady) {
    // Failed to open serial port, print some helpful instructions, then bail
    console.error('Ensure you are using the correct serial device and that the Arduino serial monitor is closed.');
    process.exit();
  }
});

// Enable sending messages to the network
udpPort.open();

// Connect to the serial port to relay messages to the Arduino
serialPort.open();
