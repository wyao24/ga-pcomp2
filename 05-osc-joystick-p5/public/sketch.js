let x, y, b;
const socket = new osc.WebSocketPort({ url: 'ws://' + window.location.host });

socket.on('message', (data) => {
  console.log(data);
  if (data.address === '/3/xy') {
    x = data.args[0] * width;
    y = data.args[1] * height;
  } else if (data.address === '/3/toggle1') {
    b = data.args[0];
  }
});

socket.open();

function setup() {
  frameRate(60);
  createCanvas(1024, 768);
  // Starts in the middle
  x = width / 2;
  y = height / 2;
}

function draw() {
  background(150);
  textSize(32);
  fill(50);
  text(x, 0, 32);
  text(y, 0, 64);

  if (b) {
    fill('red');
  }

  // Draw a circle
  stroke(50);
  ellipse(x, y, 50, 50);
}
