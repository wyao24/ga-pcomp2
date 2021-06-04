// Change this to the domain name of your Heroku server
const SERVER_URL = 'wss://change-me.herokuapp.com'

let x, y, b;
const socket = new osc.WebSocketPort({ url: SERVER_URL });

// Keep the connection alive
socket.on('close', () => {
  socket.open();
});

socket.on('error', (e) => {
  console.error('WebSocket error', e);
});

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

function mouseClicked() {
  // Do the same think whether clicked or dragged
  mouseDragged();
}

function mouseDragged() {
  const newX = constrain(mouseX / width, 0, 1);
  const newY = constrain(mouseY / height, 0, 1);
  socket.send({ address: '/3/xy', args: [newX, newY] })
}
