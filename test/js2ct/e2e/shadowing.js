function shadow_in_block() {
    let x = 1;
    if (true) {
        let x = 2;
        assert(x == 2);
    }
    assert(x == 1);
}

function first_y() {
    let y = 20;
    return y;
}

function second_y() {
    let y = 99;
    return y;
}

function dup_name() {
    return 1;
}

function calls_shadowed_dup_name() {
    function dup_name() {
        return 2;
    }
    return dup_name();
}

function run() {
    shadow_in_block();

    assert(first_y() == 20);
    assert(second_y() == 99);

    assert(dup_name() == 1);
    assert(calls_shadowed_dup_name() == 2);
    assert(dup_name() == 1);
}

run();
