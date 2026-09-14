function sumTo(n) {
    let s = 0;
    for (let i = 0; i < n; i = i + 1) {
        s = s + i;
    }
    return s;
}

function run() {
    assert(sumTo(5) == 10);
    assert(sumTo(0) == 0);
}

run();
