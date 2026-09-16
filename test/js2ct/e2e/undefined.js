function run() {
    let a;
    assert(a === undefined);

    let b;
    b = 5;
    assert(b === 5);

    assert(undefined === undefined);
    assert((undefined == 0) === false);
    assert((undefined == false) === false);
}

run();
