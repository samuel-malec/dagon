let x = +(+(+1));
assert(x === 1);

// folding isn't literal-specific -- the operand can be any expression
let n = 41;
let y = +(+n);
assert(y === 41);

// '+' folds away even when nested inside a real (non-folded) unary '-'
let z = +(-(+5));
assert(z === -5);
