var world = (function() {
    var connection = null;
    var total_bytes_sent = 0;
    var total_bytes_received = 0;

    function init() {
        connect_callbacks();
        connect_socket();
    }

    function connect_callbacks() {
        var all_callbacks = [
            ['iterate-button', 'click', iterate],
            ['reset-world-button', 'click', reset_world],
        ];

        all_callbacks.forEach(function(cb) {
            document.getElementById(cb[0]).addEventListener(cb[1], cb[2]);
        });
    }

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

    function on_receive_message(event) {
        var message = event.data;

        update_bytes_received(message.length);
        var parsed = JSON.parse(message);
        if('authenticated' in parsed) {
            display_websocket_state('Connected');
            send_message({'food_dist_requested': true});
        }

        if('food_dist' in parsed) {
            display_food_dist(parsed['food_dist']);
        }
    }

    function display_food_dist(food) {
        var canvas = document.getElementById('map-display');
        var ctx = canvas.getContext('2d');

        ctx.clearRect(0, 0, canvas.width, canvas.height);

        var x_scale = canvas.width / food['width'];
        var y_scale = canvas.height / food['height'];

        food['food'].forEach(function(col,x) {
            col.forEach(function(val, y) {
                if(val != 0) {
                    ctx.fillStyle = `rgb(${255 - 255*val}, 255, ${255 - 255*val})`
                    ctx.fillRect(x*x_scale, y*y_scale, x_scale, y_scale);
                }
            });
        });
    }

    function iterate() {
        send_message({'iterate_n_steps': 1,
                      'food_dist_requested': true});
    }

    function reset_world() {
        send_message({'reset_world': true,
                      'food_dist_requested': true});
    }

    return {init: init};
})();

world.init();
