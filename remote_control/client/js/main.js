document.addEventListener("DOMContentLoaded", function (event) {
    /* TODO: Lock orientation */

    /* Ask user to connect to the server. */
    connection_modal_show(true);

    /* Connect buttom. */
    document.getElementById("btConnect").onclick = function () {
        if (connected == false) {
            const address = document.getElementById("txtServerAddr").value;
            document.getElementById("btConnect").value = "Connecting...";
            ws_connect(address);
        } else {
            ws.close();
            connected = false;
        }
    };

    /* Request permission for iOS 13+ devices */
    if (DeviceMotionEvent && typeof DeviceMotionEvent.requestPermission === "function") {
        DeviceMotionEvent.requestPermission();
    }
    window.addEventListener('deviceorientation', handle_orientation, false);
});
