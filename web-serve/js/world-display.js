var world_state = {
    size: null,
    food_fields: null,
    creatures: null,
};

function update_world_display(message) {
    var needs_redraw = false;

    if('food_dist' in message) {
        world_state.size = message.food_dist.size;
        world_state.food_fields = message.food_dist.food_fields;
        needs_redraw = true;
    }

    if('creatures' in message) {
        world_state.creatures = message.creatures;
        needs_redraw = true;
    }

    if(needs_redraw) {
        redraw_canvas();
    }
}

function redraw_canvas() {
    var canvas = document.getElementById('map-display');
    var ctx = canvas.getContext('2d');

    ctx.clearRect(0, 0, canvas.width, canvas.height);

    draw_food_fields(ctx, world_state.food_fields);
    draw_creatures(ctx, world_state.creatures);
}



function draw_food_fields(ctx) {
    var food_fields = world_state.food_fields;
    var size = world_state.size;

    var dummy = document.createElement('canvas');
    dummy.width = 8;
    dummy.height = 8;
    var dummy_ctx = dummy.getContext('2d');

    ctx.imageSmoothingEnabled = false;

    var x_scale = ctx.canvas.width / size;
    var y_scale = ctx.canvas.height / size;

    food_fields.forEach(field => {
        for(var i=0; i<8; i++) {
            for(var j=0; j<8; j++) {
                var style = field.values[i][j] ? '#336600' : 'White';
                dummy_ctx.fillStyle = style;
                dummy_ctx.fillRect(j, 7-i, 1, 1);
            }
        }
        ctx.drawImage(dummy,
                      field.x_min*x_scale,
                      ctx.canvas.height - field.y_min*y_scale-1,
                      field.width*x_scale, -field.width*y_scale);
    });
}

function draw_creatures(ctx) {
    var width = ctx.canvas.width;
    var height = ctx.canvas.height;
    var size = world_state.size;

    world_state.creatures.forEach(function(creature) {
        var x_pixel = creature.x * width / size;
        var y_pixel = height - creature.y * height / size;
        var radius = creature.radius * width / size;

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
