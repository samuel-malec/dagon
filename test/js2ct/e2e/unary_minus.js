let a = -1;
assert(a == -1);

let b = -(-(-1));
assert(b == -1);

// unary '-' (neg) and binary '-' (sub) must not be confused with each
// other even though the parser gives them the same op_kind
let c = 5 - (-2);
assert(c == 7);

let d = -3 + 10;
assert(d == 7);
