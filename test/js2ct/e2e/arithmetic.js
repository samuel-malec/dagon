function run() {
    // precedence: '*'/'/' bind tighter than '+'/'-'
    assert(2 + 3 * 4 - 6 / 2 === 11);

    assert(3 + 4 === 7);
    assert(10 - 4 === 6);
    assert(6 * 7 === 42);
    assert(20 / 4 === 5);
    assert(7 % 3 === 1);
}

run();
