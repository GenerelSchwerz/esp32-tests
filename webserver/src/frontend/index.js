let gateway = `ws://${window.location.hostname}/ws`;
let websocket;

let readingListener = null;

// Init web socket when the page loads
window.addEventListener('load', onload);
window.addEventListener('unload', onunload);


function onload(event) {
    console.log('loading!')
    initWebSocket();
}

function onunload(event) {
    websocket.close();
    if (readingListener != null) clearInterval(readingListener);
    readingListener = null;
}

function getReadings(){
    websocket.send("0");
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

// When websocket is established, call the getReadings() function
function onOpen(event) {
    console.log('Connection opened');
    if (readingListener != null) clearInterval(readingListener)
    readingListener = setInterval(getReadings, 500);
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

// Function that receives the message from the ESP32 with the readings
function onMessage(event) {
    console.log(event.data);

    const data = JSON.parse(event.data);

    const dist = data.distance;

    const el = document.querySelector(".distance-sensor-main-display")

    el.innerHTML = `Distance: ${dist} cm`
}
