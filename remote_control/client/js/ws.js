var ws;
var connected = false;

/* Establish connection. */
function ws_connect(addr) {
    /* Connect. */
    ws = new WebSocket(addr);

    /* Register events. */
    ws.onopen = function () {
        /* Opening connection. */
        connected = true;
        document.getElementById("btConnect").value = "Connected";
        connection_modal_show(false);
    };

    /* Deals with messages. */
    ws.onmessage = function (e) {
        /* Message handling. */
        /* e.data contains message. */
    };

    /* Close events. */
    ws.onclose = function (e) {
        /* Closing connection. */
        /* e.wasClean and e.code indicate status. */
        connected = false;
        document.getElementById("btConnect").value = "Disconnected. Reconnect?";
        connection_modal_show(true);
    };
}
