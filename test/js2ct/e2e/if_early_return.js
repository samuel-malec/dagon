function classify(n) {
    if (n < 0) {
        return "negative";
    }
    if (n === 0) {
        return "zero";
    }
    return "positive";
}

function abs_guard(n) {
    if (n < 0) {
        return 0 - n;
    }
    return n;
}

function run() {
    assert(classify(-5) === "negative");
    assert(classify(0) === "zero");
    assert(classify(5) === "positive");

    assert(abs_guard(-3) === 3);
    assert(abs_guard(3) === 3);
}

run();
