function factorial(n) {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

function run() {
    assert(factorial(0) == 1);
    assert(factorial(5) == 120);
}

run();
