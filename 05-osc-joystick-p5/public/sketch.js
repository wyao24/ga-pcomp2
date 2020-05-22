let x, y, b;
const socket = io.connect(window.location.origin);

socket.on('message', (data) => {
    console.log(data);
    if (data.address === '/3/xy') {
        x = data.args[0].value * width;
        y = data.args[1].value * height;
    } else if (data.address === '/3/toggle1') {
        b = data.args[0].value;
    }
});

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

    // Draw a circle
    stroke(50);
    ellipse(x, y, 50, 50);
    // Jiggling randomly on the horizontal axis
    x = x + random(-1, 1);
    // Jiggling randomly on the vertical axis
    y = y + random(-1, 1);
}
