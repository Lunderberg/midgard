var connection = null;
var total_bytes_sent = 0;
var total_bytes_received = 0;

function display_websocket_state(state) {
    var status_span = document.getElementById("status-span");
    status_span.innerHTML = state;
}

function connect_socket() {
    display_websocket_state('Connecting');
    connection = new WebSocket('ws://' + location.host);
    connection.onopen = on_websocket_open;
    connection.onmessage = on_receive_message;
    connection.onerror = on_websocket_close;
    connection.onclose = on_websocket_close;
}

function on_websocket_open() {
    display_websocket_state('Authenticating');
    send_message({'username': 'username',
                  'password': 'password',
                 });
}

function on_websocket_close() {
    display_websocket_state('disconnected');
}

function on_receive_message(event) {
    var message = event.data;

    update_bytes_received(message.length);
    var parsed = JSON.parse(message);
    if('authenticated' in parsed) {
        display_websocket_state('Connected');
        send_message({'full_map_requested': true});
    }

    update_world_display(parsed);
}

function send_message(obj) {
    var message = JSON.stringify(obj);
    connection.send(message);
    update_bytes_sent(message.length);
}

function update_bytes_sent(bytes) {
    total_bytes_sent += bytes;
    document.getElementById('total-bytes-sent').innerHTML = format_bytes(total_bytes_sent);
}

function update_bytes_received(bytes) {
    total_bytes_received += bytes;
    document.getElementById('total-bytes-received').innerHTML = format_bytes(total_bytes_received);
}

function format_bytes(bytes) {
    if(bytes < Math.pow(1024,1)) {
        return bytes + ' bytes';
    } else if (bytes < Math.pow(1024,2)) {
        return (bytes/1024).toFixed(1) + ' kB';
    } else if (bytes < Math.pow(1024,3)) {
        return (bytes/1024/1024).toFixed(2) + ' MB';
    } else {
        return (bytes/1024/1024/1024).toFixed(3) + ' GB';
    }
}
