const internalIp = require('internal-ip');
const osc = require('osc');

// Change this to match the port you selected in the Arduino IDE
const SERIAL_DEVICE = '/dev/cu.usbmodem14201';

// You shouldn't need to change these
const LOCAL_ADDRESS = internalIp.v4.sync() || '0.0.0.0';
const LOCAL_PORT = 8888;
const SERIAL_BITRATE = 57600;

// Set up the network port
const udpPort = new osc.UDPPort({
  localAddress: LOCAL_ADDRESS,
  localPort: LOCAL_PORT,
  metadata: true,
});

// Set up the serial port to talk to the Arduino
const serialPort = new osc.SerialPort({
  bitrate: SERIAL_BITRATE,
  devicePath: SERIAL_DEVICE,
  metadata: true,
});

// Once our network port is ready, start listening for messages and print our IP address
udpPort.on('ready', () => {
  // When messages arrive, print them to the console and send them over the serial port
  udpPort.on('message', (msg) => {
    console.log('Network to device:', msg);
    serialPort.send(msg);
  });

  console.log('Listening on', LOCAL_ADDRESS, 'port', LOCAL_PORT);
});

// Once we've connected to serial successfully, start listening for messages
serialPort.on('ready', () => {
  // When messages arrive, print them to the console and send them over the network
  serialPort.on('message', (msg) => {
    console.log('Device to network:', msg);
    udpPort.send(msg);
  })

  console.log('Connected to serial device', SERIAL_DEVICE);
})

// Catch errors on both ports
udpPort.on('error', (error) => {
  console.error(error.message || error);
  console.error('There may be another server running on this port.');
  process.exit();
});

serialPort.on('error', (error) => {
  console.error(error.message || error);
  console.error('Ensure you are using the correct serial device and that the Arduino serial monitor is closed.');
  process.exit();
});

// Start listening for messages from the network
udpPort.open();

// Connect to the serial port to relay messages to the Arduino
serialPort.open();
