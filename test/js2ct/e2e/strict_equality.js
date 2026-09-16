function run() {
    assert((1 == "1") === true);
    assert((1 === "1") === false);
    assert((1 !== "1") === true);
    assert((1 != "1") === false);

    // same-typed values: '===' behaves like '=='
    assert((5 === 5) === true);
    assert((5 !== 5) === false);
    assert(("foo" === "foo") === true);
    assert(("foo" === "bar") === false);
}

run();
