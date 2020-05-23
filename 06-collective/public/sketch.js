const socket = new osc.WebSocketPort({ url: 'ws://' + window.location.host });

socket.on('message', (msg) => {
  console.log(msg);
});

socket.open();

function setup() {
  frameRate(60);
  createCanvas(1024, 768);
}

function draw() {
  background(150);
  textSize(32);
  fill(50);
}
