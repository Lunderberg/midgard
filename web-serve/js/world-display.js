function update_world_display(message) {
    if('food_dist' in message) {
        display_food_dist(message['food_dist']);
    }

    if('creatures' in message) {
        display_creatures(message['creatures']);
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
