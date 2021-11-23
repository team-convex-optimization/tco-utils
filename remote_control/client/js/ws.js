var ws;
var ws_connected = false;
var TCO_VIDEO_HEIGHT = 480;
var TCO_VIDEO_WIDTH = 640;

/* For image processing */
const canvas = document.getElementById("canvas");
const ctx = canvas.getContext('2d');

/* Establish connection. */
function ws_connect(addr) {
    /* Connect. */
    ws = new WebSocket(addr);
    ws.binaryType = "arraybuffer"; /* Decide how to register event */
    /* Register events. */
    ws.onopen = function () {
        /* Opening connection. */
        ws_connected = true;
        document.getElementById("btConnect").value = "Connected";
        connection_modal_show(false);
        /* Set video object */
        ctx.width = TCO_VIDEO_WIDTH;
        ctx.height = TCO_VIDEO_HEIGHT;
    };

    /* Deals with messages. AKA if video is sent, It will receive it */
    ws.onmessage = function (e) {
        var inputBytes = new Uint8Array(e.data);
        let imgData = ctx.getImageData(0, 0, ctx.canvas.width, ctx.canvas.height);
        let pixels = imgData.data;

        /* HTML5 Canvas uses 4 color channels */
        for (var i = 0; i < TCO_VIDEO_HEIGHT*TCO_VIDEO_WIDTH*4; i+=4) { 
            pixels[i] = (inputBytes[i/4]);
            pixels[i+1] = (inputBytes[i/4]);
            pixels[i+2] = (inputBytes[i/4]);
            pixels[i+3] = (inputBytes[i/4]);
        }
        ctx.putImageData(imgData, 0, 0); /* Write the image */
    };

    /* Close events. */
    ws.onclose = function (e) {
        /* Closing connection. */
        /* e.wasClean and e.code indicate status. */
        ws_connected = false;
        document.getElementById("btConnect").value = "Disconnected. Reconnect?";
        connection_modal_show(true);
    };
}
