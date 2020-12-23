const horiz_line = document.getElementById("indHorz");
const vert_line = document.getElementById("indVert");

function handle_orientation(event) {
    const width = window.innerWidth;
    const height = window.innerHeight;
    const rng_hlf = 30; // Degrees for full range of values
    const gamma_offset = -30; // Degrees

    /* In degrees, in range [-180, 180) */
    let x = event.beta;
    let y = event.gamma;
    y += gamma_offset;

    /* Range to [-rng_hlf, rng_hlf] */
    if (x < -rng_hlf) { x = -rng_hlf; }
    if (x > rng_hlf) { x = rng_hlf; }
    if (y < -rng_hlf) { y = -rng_hlf; }
    if (y > rng_hlf) { y = rng_hlf; }

    update_throttle((-y / rng_hlf) * 100);
    update_steering((-x / rng_hlf) * 100);

    document.getElementById("orientValX").innerHTML = Math.round(x) + '°';
    document.getElementById("orientValY").innerHTML = Math.round(y) + '°';

    horiz_line.style.top = ((height * y) / (2 * rng_hlf)) + "px";
    vert_line.style.left = ((width * -x) / (2 * rng_hlf)) + "px";
}

function update_throttle(throttle) {
    document.getElementById("throttleVal").innerHTML = Math.round(throttle) + '%';
    document.getElementById("throttleValDrv").innerHTML = Math.round(throttle);
    ws.send(`0 ${Math.round(Number(throttle.toFixed(4)) * 10000)}\n`);
}

function update_steering(steering) {
    document.getElementById("steeringVal").innerHTML = Math.round(steering) + '%';
    document.getElementById("steeringValDrv").innerHTML = Math.round(steering);
    ws.send(`1 ${Math.round(Number(steering.toFixed(4)) * 10000)}\n`);
}