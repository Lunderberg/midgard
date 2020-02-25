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
            send_message({'full_map_requested': true});
        }

        if('food_dist' in parsed) {
            display_food_dist(parsed['food_dist']);
        }

        if('creatures' in parsed) {
            display_creatures(parsed['creatures']);
        }
    }

    function display_food_dist(food) {
        var canvas = document.getElementById('map-display');
        var ctx = canvas.getContext('2d');

        var dummy = document.createElement('canvas');
        dummy.width = 8;
        dummy.height = 8;
        var dummy_ctx = dummy.getContext('2d');

        ctx.clearRect(0, 0, canvas.width, canvas.height);
        ctx.imageSmoothingEnabled = false;

        var x_scale = canvas.width / food['size'];
        var y_scale = canvas.height / food['size'];

        food['food_fields'].forEach(field => {
            for(var i=0; i<8; i++) {
                for(var j=0; j<8; j++) {
                    var style = field.values[i][j] ? '#336600' : 'White';
                    dummy_ctx.fillStyle = style;
                    dummy_ctx.fillRect(j, 7-i, 1, 1);
                }
            }
            ctx.drawImage(dummy,
                          field.x_min*x_scale,
                          canvas.height - field.y_min*y_scale-1,
                          field.width*x_scale, -field.width*y_scale);
        });
    }

    function display_creatures(creatures) {
        var canvas = document.getElementById('map-display');
        var ctx = canvas.getContext('2d');

        creatures.forEach(function(creature) {
            // TODO: Make dedicated structure for remembering world
            // info, including size.
            var x_pixel = creature.x * canvas.width / 64;
            var y_pixel = canvas.height - creature.y * canvas.height / 64;
            var radius = creature.radius * canvas.width / 64;

            ctx.beginPath();
            ctx.arc(x_pixel, y_pixel, radius, 0, 2*Math.PI, false);
            ctx.fillStyle = 'rgba(255, 255, 255, 0.7)';
            ctx.fill();
            ctx.lineWidth = 3;
            ctx.strokeStyle = 'black';
            ctx.stroke();

            ctx.beginPath();
            ctx.lineWidth = 3;
            ctx.strokeStyle = 'black';
            ctx.moveTo(x_pixel, y_pixel);
            ctx.lineTo(x_pixel + radius*Math.cos(creature.direction), y_pixel - radius*Math.sin(creature.direction));
            ctx.stroke();
        });
    }

    function iterate() {
        var num_iter = +document.getElementById('num-iterations').value;
        send_message({'iterate_n_steps': num_iter,
                      'food_dist_requested': true});
    }

    function reset_world() {
        send_message({'reset_world': true,
                      'food_dist_requested': true});
    }

    return {init: init};
})();

world.init();
