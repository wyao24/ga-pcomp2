const osc = require('osc');

// Change this to match the port you selected in the Arduino IDE
const SERIAL_DEVICE = '/dev/cu.usbmodem14201';

// Change this to match the server you're trying to talk to
const SERVER_URL = 'wss://change-me.herokuapp.com';

// You shouldn't need to change this
const SERIAL_BITRATE = 57600;

// Connect to the server via WebSocket
const webSocketPort = new osc.WebSocketPort({ url: SERVER_URL });

// Set up the serial port to talk to the Arduino
const serialPort = new osc.SerialPort({
  bitrate: SERIAL_BITRATE,
  devicePath: SERIAL_DEVICE,
});

let serialPortReady = false;

// Once we've connected to the serial port, start forwarding messages
serialPort.on('ready', () => {
  console.log('Connected to serial device', serialPort.options.devicePath);
  serialPortReady = true;

  serialPort.on('message', (msg) => {
    console.log('Device to web:', msg);
    webSocketPort.send(msg);
  });

  // Start listening for messages from the server and forwarding them to the serial port
  webSocketPort.on('message', (msg) => {
    console.log('Web to device:', msg);
    serialPort.send(msg);
  });
});

// Catch errors on all ports
webSocketPort.on('error', (error) => {
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

// Get everything going
serialPort.open();
webSocketPort.open();
