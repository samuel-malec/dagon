function run() {
    // body runs at least once even when the condition starts false
    let i = 0;
    let n = 0;
    do {
        n = n + 1;
        i = i + 1;
    } while (i < 0);
    assert(n === 1);

    let s = 0;
    let j = 0;
    do {
        s = s + j;
        j = j + 1;
    } while (j < 5);
    assert(s === 10);
}

run();
