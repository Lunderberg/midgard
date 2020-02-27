function connect_callbacks() {
    var all_callbacks = [
        ['iterate-button', 'click', iterate],
        ['reset-world-button', 'click', reset_world],
    ];

    all_callbacks.forEach(function(cb) {
        document.getElementById(cb[0]).addEventListener(cb[1], cb[2]);
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
