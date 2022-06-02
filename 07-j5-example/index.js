// See http://johnny-five.io/examples/repl/

const five = require('johnny-five');
const board = new five.Board();

board.on('ready', () => {
  const led = new five.Led(13);

  board.repl.inject({
    // Allow limited on/off control access to the Led instance from the command line.
    on: () => {
      led.on();
    },
    off: () => {
      led.off();
    },
  });
});
