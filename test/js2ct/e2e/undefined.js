function run() {
    // 'let x;' (no initializer) means 'let x = undefined;', same as real JS
    let a;
    assert(a === undefined);

    // still assignable afterward -- declaring uninitialized isn't final
    let b;
    b = 5;
    assert(b === 5);

    // the 'undefined' literal itself
    assert(undefined === undefined);
    assert((undefined == 0) === false);
    assert((undefined == false) === false);
}

run();
