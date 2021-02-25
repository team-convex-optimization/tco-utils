let interval_update_loop_keyboard;
let interval_update_loop_gamepad;
let input_mode = "none";

document.getElementById("btConnect").addEventListener("click", function (event) {
    // Connect to server
    if (ws_connected == false) {
        const address = document.getElementById("txtServerAddr").value;
        document.getElementById("btConnect").value = "Connecting...";
        ws_connect(address);
    } else {
        ws.close();
        ws_connected = false;
    }

    let input_mode_selected;
    const radio_selector = document.getElementsByName("radioControlsSelect");
    for (var i = 0, length = radio_selector.length; i < length; i++) {
        if (radio_selector[i].checked) {
            input_mode_selected = radio_selector[i].value;
            break;
        }
    }

    // First cleanup the old input method
    if (input_mode != "none" && input_mode_selected != input_mode) {
        switch (input_mode) {
            case "sensors":
                window.removeEventListener('deviceorientation', handle_orientation, false);
                break;
            case "keyboard":
                window.removeEventListener('keydown', handle_keydown, false);
                window.removeEventListener('keyup', handle_keyup, false);
                clearInterval(interval_update_loop_keyboard);
                interval_update_loop_keyboard = null;
                input_mode = "none";
                break;
            case "gamepad":
                window.removeEventListener('gamepadconnected', connect_gamepad, false);
                window.removeEventListener('gamepaddisconnected', disconnect_gamepad, false);
                clearInterval(interval_update_loop_gamepad);
                interval_update_loop_gamepad = null;
                input_mode = "none";
                break;

            default:
                console.log("Unknown input mode");
                break;
        }
    }
    // Init the input method selected
    if (input_mode == "none")
        switch (input_mode_selected) {
            case "sensors":
                /* Request permission for iOS 13+ devices */
                if (DeviceMotionEvent && typeof DeviceMotionEvent.requestPermission === "function") {
                    DeviceMotionEvent.requestPermission();
                }
                window.addEventListener('deviceorientation', handle_orientation, false);
                input_mode = "sensors";
                break;

            case "keyboard":
                window.addEventListener('keydown', handle_keydown, false);
                window.addEventListener('keyup', handle_keyup, false);
                interval_update_loop_keyboard = setInterval(update_loop_keyboard, 16); // 60 fps
                input_mode = "keyboard";
                break;

            case "gamepad":
                window.addEventListener('gamepadconnected', connect_gamepad, false);
                window.addEventListener('gamepaddisconnected', disconnect_gamepad, false);
                interval_update_loop_gamepad = setInterval(update_loop_gamepad, 8); // 120 fps
                input_mode = "gamepad";
                break;

            default:
                console.log("Unknown input mode");
                break;
        }
});

window.onerror = function (msg, url, linenumber) {
    alert('Error message: ' + msg + '\nURL: ' + url + '\nLine Number: ' + linenumber);
    return true;
}
