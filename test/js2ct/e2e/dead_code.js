function classify(x) {
    if (x < 0) {
        return 0 - x;
    } else {
        return x + 100;
    }
    let ignored = x + 1;
    return ignored;
}

function afterReturn(x) {
    let y = x * 2;
    return y;
    let z = y + 1;
    return z;
}

function guarded(x) {
    if (x === 0) {
        return 42;
    }
    let scaled = x * 3;
    return scaled;
}

function run() {
    assert(classify(0 - 3) === 3);
    assert(classify(3) === 103);
    assert(afterReturn(5) === 10);
    assert(guarded(0) === 42);
    assert(guarded(4) === 12);
}

run();
