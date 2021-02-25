let status_keypress = { 'w': false, 'a': false, 's': false, 'd': false, 'alt': false };

/* Used to make keyboard steering smoother. */
let last_throttle = 0.0;
let last_steering = 0.0;

/* Reference to the connected gamepad */
let gamepad_index = null;

const line_horiz = document.getElementById("indHorz");
const line_vert = document.getElementById("indVert");

function handle_orientation(event) {
    const rng_hlf = 30; // Degrees for full range of values
    const offset_gamma = -30; // Degrees

    /* In degrees, in range [-180, 180) */
    let x = event.beta;
    let y = event.gamma;
    y += offset_gamma;

    /* Range to [-rng_hlf, rng_hlf] */
    if (x < -rng_hlf) { x = -rng_hlf; }
    if (x > rng_hlf) { x = rng_hlf; }
    if (y < -rng_hlf) { y = -rng_hlf; }
    if (y > rng_hlf) { y = rng_hlf; }

    update_throttle((-y / rng_hlf) * 100);
    update_steering((-x / rng_hlf) * 100);

    document.getElementById("orientValX").innerHTML = Math.round(x) + '°';
    document.getElementById("orientValY").innerHTML = Math.round(y) + '°';
}

function handle_keydown(event) {
    switch (event.key) {
        case 'w':
        case 'a':
        case 's':
        case 'd':
            status_keypress[event.key] = true;
            if (event.key.toUpperCase() != event.key) {
                status_keypress[event.key.toUpperCase()] = false;
            }
            break;
        case String.fromCharCode(32):
            /* Pressing space resets controls. */
            last_throttle = 0;
            last_steering = 0;
            update_throttle(0);
            update_steering(0);
        default:
            break;
    }
    if (event.altKey) {
        status_keypress['alt'] = true;
    }
}

function handle_keyup(event) {
    switch (event.key) {
        case 'w':
        case 'a':
        case 's':
        case 'd':
            status_keypress[event.key] = false;
            if (event.key.toUpperCase() != event.key) {
                status_keypress[event.key.toUpperCase()] = false;
            }
            break;
        default:
            break;
    }
    if (event.altKey) {
        status_keypress['alt'] = false;
    }
}

function connect_gamepad(event) {
    console.log("Gamepad connected at index %d: %s. %d buttons, %d axes.",
        event.gamepad.index, event.gamepad.id,
        event.gamepad.buttons.length, event.gamepad.axes.length
    );
    gamepad_index = event.gamepad.index;
}

function disconnect_gamepad(event) {
    console.log("Gamepad disconnected from index %d: %s",
        event.gamepad.index, event.gamepad.id
    );
    delete gamepads[event.gamepad.index];
    gamepad_index = null;
    mode_controller = false;
}

function update_loop_gamepad() {
    // Only run update loop when a gamepad was connected
    if (gamepad_index != null) {
        const throttle_button = 7;
        const brake_button = 6;
        const steering_axis = 0;

        let next_throttle = 0;
        let next_steering = 0;

        const gp = navigator.getGamepads()[gamepad_index];
        next_throttle = ((gp.buttons[throttle_button].value) - gp.buttons[brake_button].value) * 100;
        next_steering = gp.axes[steering_axis] * 100;

        update_throttle(next_throttle);
        update_steering(next_steering);
    }
}

function update_loop_keyboard() {
    let next_throttle = 0;
    let next_steering = 0;
    let now_steering = 0;
    let now_throttle = 0;
    if (status_keypress['w']) {
        if (status_keypress['alt']) {
            now_throttle += 3;
        }
        else {
            now_throttle += 1;
        }
    }
    if (status_keypress['s']) {
        if (status_keypress['alt']) {
            now_throttle -= 3;
        }
        else {
            now_throttle -= 1;
        }
    }
    if (status_keypress['a']) {
        now_steering -= 100;
    }
    if (status_keypress['d']) {
        now_steering += 100;
    }

    /* Apply new values. */
    next_throttle = last_throttle + now_throttle;
    const weight_new_steering = 0.20;
    next_steering = ((1 - weight_new_steering) * last_steering) + (weight_new_steering * now_steering);

    if (next_throttle > 100) next_throttle = 100;
    if (next_throttle < -100) next_throttle = -100;
    if (next_steering > 100) next_steering = 100;
    if (next_steering < -100) next_steering = -100;

    last_throttle = next_throttle;
    last_steering = next_steering;

    update_throttle(next_throttle);
    update_steering(next_steering);
}


function update_throttle(throttle) {
    document.getElementById("throttleVal").innerHTML = Math.round(throttle) + '%';
    document.getElementById("throttleValDrv").innerHTML = Math.round(throttle);
    if (ws_connected == true) {
        ws.send(`0 ${Math.round(Number(throttle.toFixed(4)) * 10000)}\n`);
    }
    const height = window.innerHeight;
    line_horiz.style.top = ((height * -throttle) / (2 * 100)) + "px";
}

function update_steering(steering) {
    document.getElementById("steeringVal").innerHTML = Math.round(steering) + '%';
    document.getElementById("steeringValDrv").innerHTML = Math.round(steering);
    if (ws_connected == true) {
        ws.send(`1 ${Math.round(Number(steering.toFixed(4)) * 10000)}\n`);
    }
    const width = window.innerWidth;
    line_vert.style.left = ((width * steering) / (2 * 100)) + "px";
}
