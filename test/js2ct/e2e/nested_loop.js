// Nested loops carrying several live bindings across each iteration: this is
// what makes a loop body return more than one value in hicthu, before
// hicthu2locthu packs them into an array for QuickJS.
function grid(rows, cols) {
    let r = 0;
    let total = 0;
    let cells = 0;
    while (r < rows) {
        let c = 0;
        while (c < cols) {
            total = total + r * cols + c;
            cells = cells + 1;
            c = c + 1;
        }
        r = r + 1;
    }
    return total * 100 + cells;
}

function countdown(n) {
    let acc = 0;
    let steps = 0;
    let flip = 0;
    while (n > 0) {
        if (flip === 0) {
            acc = acc + n;
            flip = 1;
        } else {
            acc = acc - 1;
            flip = 0;
        }
        steps = steps + 1;
        n = n - 1;
    }
    return acc * 10 + steps;
}

function run() {
    // 3x4 grid: sum of 0..11 is 66, 12 cells
    assert(grid(3, 4) === 6612);
    // 1x1 grid: single cell holding 0
    assert(grid(1, 1) === 1);
    // rows = 0 never enters the outer loop
    assert(grid(0, 5) === 0);

    // n=4: +4, -1, +2, -1 => 4, in 4 steps
    assert(countdown(4) === 44);
    assert(countdown(0) === 0);
}

run();
