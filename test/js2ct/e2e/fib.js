function fib_rec(fib1, fib2, n) {
    if (n == 0) {
        return fib2;
    }
    return fib_rec(fib2, fib1 + fib2, n - 1);
}

function fib_base(n) {
    if (n <= 2) {
        return 1;
    }
    return fib_rec(1, 1, n - 2);
}

assert( fib_base( 1 ) == 1 );
assert( fib_base( 2 ) == 1 );
assert( fib_base( 3 ) == 2 );
assert( fib_base( 4 ) == 3 );
assert( fib_base( 5 ) == 5 );
