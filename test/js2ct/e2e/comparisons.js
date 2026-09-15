function run() {
    assert((3 <= 3) === true);
    assert((3 <= 2) === false);
    assert((5 > 3) === true);
    assert((3 > 5) === false);
    assert((3 >= 3) === true);
    assert((2 >= 3) === false);
    assert((3 !== 4) === true);
    assert((3 !== 3) === false);
}

run();
