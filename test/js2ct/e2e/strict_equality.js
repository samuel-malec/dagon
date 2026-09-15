function run() {
    // '===' does not coerce types -- unlike '=='. The inner comparisons
    // here are the whole point of the test, so '==' / '!=' stay as-is
    // deliberately -- converting them to '===' / '!==' would make this
    // fixture stop testing coercion at all.
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
