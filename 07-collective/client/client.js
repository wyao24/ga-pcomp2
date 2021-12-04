const osc = require('osc');
const readline = require('readline');

const SERVER_ADDRESS = '127.0.0.1';
const SERVER_UDP_PORT = 9000;

const client = new osc.UDPPort({
  remoteAddress: SERVER_ADDRESS,
  remotePort: SERVER_UDP_PORT,
  metadata: true,
});

const input = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
});

async function askForHue() {
  return new Promise((resolve) => {
    input.question('Hue = ', (answer) => {
      let hue = Number.parseInt(answer, 10);

      if (Number.isNaN(hue) || hue < 0 || hue > 255) {
        hue = Math.floor(Math.random() * 256);
        console.log('Using random value', hue);
      }
      resolve(hue);
    });
  });
}

async function askForPosition(prompt) {
  return new Promise((resolve) => {
    input.question(prompt, (answer) => {
      let position = Number.parseFloat(answer);
      if (Number.isNaN(position) || position < 0 || position > 1) {
        position = 0.5;
        console.log('Using default value', position);
      }
      resolve(position);
    });
  });
}


async function askYesNo(prompt) {
  return new Promise((resolve) => {
    input.question(prompt, (answer) => {
      const firstChar = answer.toUpperCase()[0];
      let result;

      if (firstChar === 'Y') {
        result = 1;
      } else {
        result = 0;
        if (firstChar !== 'N') {
          console.log('Using default value', result);
        }
      }
      resolve(result);
    });
  });
}

function sendMessage(hue, x, y, button) {
  client.send({
    address: '/xy',
    args: [
      { type: 'f', value: x },
      { type: 'f', value: y },
      { type: 'i', value: hue },
    ]
  });
  console.log('->', SERVER_ADDRESS, '/xy', [x, y, hue]);

  client.send({
    address: '/toggle',
    args: [
      { type: 'f', value: button },
      { type: 'i', value: hue },
    ]
  });
  console.log('->', SERVER_ADDRESS, '/toggle', [button, hue]);
}

client.on('ready', async () => {
  console.log('Enter values as prompted. Hit enter to use the default value.');

  const hue = await askForHue();

  while (true) {
    console.log();
    const x = await askForPosition('X = ');
    const y = await askForPosition('Y = ');
    const button = await askYesNo('Button on [y/N]? ');

    sendMessage(hue, x, y, button);
  }
});

client.open();
