function run() {
    let s = "hello";
    assert(s == "hello");
    assert(s != "world");
    assert(("foo" == "foo") == true);
    assert(("foo" == "bar") == false);

    let cat = "foo" + "bar";
    assert(cat == "foobar");
}

run();
