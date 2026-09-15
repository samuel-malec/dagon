# JS dialect showcase: variables, objects, arrays, and functions

This document walks through small JavaScript programs and the actual
Cthulhu IR (`.ct`) that `js2ct` generates for them.

Each example is a real, checked-in, `assert`-verified fixture. Source pairs:

| JS source                                                | Demonstrates                                                                               |
|----------------------------------------------------------|--------------------------------------------------------------------------------------------|
| [reassign.js](../test/js2ct/e2e/reassign.js)             | reassigning an existing variable to a new value                                            |
| [field_mutation.js](../test/js2ct/e2e/field_mutation.js) | mutating a field on an object *without* reassigning the variable that holds it             |
| [point.js](../test/js2ct/e2e/point.js)                   | object construction, string-keyed fields, a function call                                  |
| [box.js](../test/js2ct/e2e/box.js)                       | an array nested inside an object                                                           |
| [sum_array.js](../test/js2ct/e2e/sum_array.js)           | array iteration via a `while` loop                                                         |
| [counter.js](../test/js2ct/e2e/counter.js)               | a loop that mutates an object field, across two function calls, plus a reassigned variable |

Run them yourself:

```bash
test/run_js2ct.sh
```

`run_js2ct.sh` chains `js2ct` → `ct2qjs` → `bcrun` for every fixture in
`test/js2ct/e2e/`; each one ends in a JS-level `assert(...)` that fails the
process (nonzero exit) if the computed answer is wrong. To see the Cthulhu
IR and bytecode for any one of them yourself, instead of trusting the text
below:

```bash
test/explore_js2ct.sh test/js2ct/e2e/point.js
```

## What is supported

- Literals & types: `int`, `bool`, `string` — all as a single boxed jsvalue, no real type distinctions beyond that.
- Operators: `+ - * / %`, unary `+/-`, `== != === !== < <= > >=`, `&& ||` (short-circuit), `!`, `& | ^ ~ << >>`.
- Variables: let, reassignment, compound assignment
- Control flow: if/else (tail and non-tail position, including early-return guard clauses), while, do-while, for.
- Functions: declarations, calls, any number of params, recursion.
- Objects/arrays: literals with inline values, dot and bracket access, mutation, array push/pop via index assignment.
- Strings: literals, equality/inequality, + concatenation.

## Notable omissions

- `break` and `continue`
- closures
- classes, switch, try/catch, arrow functions, typeof/instanceof
- computed object keys {[expr]: v} and multi-level assignment targets (a.b.c = v)

## 1. Reassigning a variable — `reassign`

```js
function run() {
    let x = 1;
    x = x + 1;
    assert(x == 2);
}

run();
```

```
structure main
(
    run = λ
    (
        run run → f_ref1
        f__j call f_ref1 → %0
    )
)
structure run
(
    run = λ
    (
        jsvalue cons_1 → %0
        jsvalue dup %0 → %1 %2
        jsvalue cons_1 → %3
        jsvalue add %1 %3 → %4
        jsvalue drop → %2
        jsvalue copy %4 → %2
        jsvalue dup %2 → %5 %6
        jsvalue dup %6 → %7 %8
        jsvalue cons_2 → %9
        jsvalue eq? %7 %9 → %10
        jsvalue assert %10
        jsvalue drop → %8
    )
)
```

## 2. Mutating a field without reassigning the variable — `field_mutation`

```js
function run() {
    let obj = {};
    obj.x = 1;
    obj.x = obj.x + 1;
    assert(obj.x == 2);
}

run();
```

```
structure main
(
    run = λ
    (
        run run → f_ref1
        f__j call f_ref1 → %0
    )
)
structure run
(
    run = λ
    (
        jsvalue cons_obj → %0
        jsvalue cons_str "x" → %1
        jsvalue cons_1 → %2
        jsvalue dup %2 → %3 %4
        jsvalue set %0 %1 %3 → %5
        jsvalue cons_str "x" → %6
        jsvalue dup %5 → %7 %8
        jsvalue cons_str "x" → %9
        jsvalue get %7 %9 → %10
        jsvalue cons_1 → %11
        jsvalue add %10 %11 → %12
        jsvalue dup %12 → %13 %14
        jsvalue set %8 %6 %13 → %15
        jsvalue dup %15 → %16 %17
        jsvalue cons_str "x" → %18
        jsvalue get %16 %18 → %19
        jsvalue cons_2 → %20
        jsvalue eq? %19 %20 → %21
        jsvalue assert %21
        jsvalue drop → %17
    )
)
```

## 3. Object construction + a function call — `point`

```js
function makePoint(x, y) {
    let p = {};
    p.x = x;
    p.y = y;
    return p;
}

function run() {
    let p = makePoint(3, 4);
    assert(p.x + p.y == 7);
}

run();
```

```
structure main
(
    run = λ
    (
        run run → f_ref1
        f__j call f_ref1 → %0
    )
)
structure makePoint
(
    run = λ %0 %1 → out
    (
        jsvalue cons_obj → %2
        jsvalue cons_str "x" → %3
        jsvalue dup %0 → %4 %5
        jsvalue dup %4 → %6 %7
        jsvalue set %2 %3 %6 → %8
        jsvalue cons_str "y" → %9
        jsvalue dup %1 → %10 %11
        jsvalue dup %10 → %12 %13
        jsvalue set %8 %9 %12 → %14
        jsvalue dup %14 → %15 %16
        jsvalue move %15 → out
        jsvalue drop → %16
        jsvalue drop → %11
        jsvalue drop → %5
    )
)
structure run
(
    run = λ
    (
        jsvalue cons_3 → %0
        jsvalue cons_4 → %1
        makePoint run → f_ref1
        f_j2_j call f_ref1 %0 %1 → %2
        jsvalue dup %2 → %3 %4
        jsvalue cons_str "x" → %5
        jsvalue get %3 %5 → %6
        jsvalue dup %4 → %7 %8
        jsvalue cons_str "y" → %9
        jsvalue get %7 %9 → %10
        jsvalue add %6 %10 → %11
        jsvalue cons_7 → %12
        jsvalue eq? %11 %12 → %13
        jsvalue assert %13
        jsvalue drop → %8
    )
)
```

## 4. An array nested inside an object — `box`

```js
function makeBox() {
    let b = {};
    let items = [];
    items[0] = 100;
    items[1] = 200;
    b.items = items;
    return b;
}

function run() {
    let b = makeBox();
    let items = b.items;
    assert(items[0] + items[1] == 300);
}

run();
```

```
structure main
(
    run = λ
    (
        run run → f_ref1
        f__j call f_ref1 → %0
    )
)
structure makeBox
(
    run = λ → out
    (
        jsvalue cons_obj → %0
        jsvalue cons_arr → %1
        jsvalue cons_0 → %2
        jsvalue cons_100 → %3
        jsvalue dup %3 → %4 %5
        jsvalue set %1 %2 %4 → %6
        jsvalue cons_1 → %7
        jsvalue cons_200 → %8
        jsvalue dup %8 → %9 %10
        jsvalue set %6 %7 %9 → %11
        jsvalue cons_str "items" → %12
        jsvalue dup %11 → %13 %14
        jsvalue dup %13 → %15 %16
        jsvalue set %0 %12 %15 → %17
        jsvalue dup %17 → %18 %19
        jsvalue move %18 → out
        jsvalue drop → %14
        jsvalue drop → %19
    )
)
structure run
(
    run = λ
    (
        makeBox run → f_ref1
        f__j call f_ref1 → %0
        jsvalue dup %0 → %1 %2
        jsvalue cons_str "items" → %3
        jsvalue get %1 %3 → %4
        jsvalue dup %4 → %5 %6
        jsvalue cons_0 → %7
        jsvalue get %5 %7 → %8
        jsvalue dup %6 → %9 %10
        jsvalue cons_1 → %11
        jsvalue get %9 %11 → %12
        jsvalue add %8 %12 → %13
        jsvalue cons_300 → %14
        jsvalue eq? %13 %14 → %15
        jsvalue assert %15
        jsvalue drop → %10
        jsvalue drop → %2
    )
)
```

## 5. Array iteration — `sum_array`

```js
function sumArray(arr, n) {
    let i = 0;
    let total = 0;
    while (i < n) {
        total = total + arr[i];
        i = i + 1;
    }
    return total;
}

function run() {
    let a = [];
    a[0] = 10;
    a[1] = 20;
    a[2] = 30;
    assert(sumArray(a, 3) == 60);
}

run();
```

```
structure main
(
    run = λ
    (
        run run → f_ref1
        f__j call f_ref1 → %0
    )
)
structure sumArray
(
    loop1 = λ %3 %2 %1 %0 → packed
    (
        jsvalue dup %2 → %4 %5
        jsvalue dup %1 → %6 %7
        jsvalue lt? %4 %6 → %8
        jsvalue dup %8 → cmp18 cmp19
        jsvalue not cmp19 → cmp20
        sumArray loopbody2 → ref21
        sumArray loopexit3 → ref22
        sumArray loopframe4 → ref23
        f_j4_j opt cmp18 ref21 → alt24
        f_j4_j opt cmp20 ref22 → alt25
        f_j4_j join alt24 alt25 ref23 → cont26
        f_j4_j call cont26 %3 %5 %7 %0 → packed
    )
    loopbody2 = λ %3 %2 %1 %0 → packed
    (
        jsvalue dup %3 → %9 %10
        jsvalue dup %0 → %11 %12
        jsvalue dup %2 → %13 %14
        jsvalue get %11 %13 → %15
        jsvalue add %9 %15 → %16
        jsvalue drop → %10
        jsvalue copy %16 → %10
        jsvalue dup %10 → %17 %18
        jsvalue dup %14 → %19 %20
        jsvalue cons_1 → %21
        jsvalue add %19 %21 → %22
        jsvalue drop → %20
        jsvalue copy %22 → %20
        jsvalue dup %20 → %23 %24
        sumArray loop1 → self14
        f_j4_j call self14 %18 %24 %1 %12 → packed
    )
    loopexit3 = λ %3 %2 %1 %0 → arr13
    (
        jsvalue cons_arr → arr5
        jsvalue cons_0 → k6
        jsvalue set arr5 k6 %3 → arr7
        jsvalue cons_1 → k8
        jsvalue set arr7 k8 %2 → arr9
        jsvalue cons_2 → k10
        jsvalue set arr9 k10 %1 → arr11
        jsvalue cons_3 → k12
        jsvalue set arr11 k12 %0 → arr13
    )
    loopframe4 = λ A B %3 %2 %1 %0 → packed17
    (
        jsvalue dup %3 → %3_1 %3_2
        jsvalue dup %2 → %2_1 %2_2
        jsvalue dup %1 → %1_1 %1_2
        jsvalue dup %0 → %0_1 %0_2
        f_j4_j call A %3_1 %2_1 %1_1 %0_1 → p115
        f_j4_j call B %3_2 %2_2 %1_2 %0_2 → p216
        jsvalue join p115 p216 → packed17
    )
    run = λ %0 %1 → out
    (
        jsvalue cons_0 → %2
        jsvalue cons_0 → %3
        sumArray loop1 → ref27
        f_j4_j call ref27 %3 %2 %1 %0 → packed28
        jsvalue dup packed28 → u29 u30
        jsvalue cons_0 → k31
        jsvalue get u29 k31 → %25
        jsvalue dup u30 → u32 u33
        jsvalue cons_1 → k34
        jsvalue get u32 k34 → %26
        jsvalue dup u33 → u35 u36
        jsvalue cons_2 → k37
        jsvalue get u35 k37 → %27
        jsvalue cons_3 → k38
        jsvalue get u36 k38 → %28
        jsvalue dup %25 → %29 %30
        jsvalue move %29 → out
        jsvalue drop → %30
        jsvalue drop → %26
        jsvalue drop → %27
        jsvalue drop → %28
    )
)
structure run
(
    run = λ
    (
        jsvalue cons_arr → %0
        jsvalue cons_0 → %1
        jsvalue cons_10 → %2
        jsvalue dup %2 → %3 %4
        jsvalue set %0 %1 %3 → %5
        jsvalue cons_1 → %6
        jsvalue cons_20 → %7
        jsvalue dup %7 → %8 %9
        jsvalue set %5 %6 %8 → %10
        jsvalue cons_2 → %11
        jsvalue cons_30 → %12
        jsvalue dup %12 → %13 %14
        jsvalue set %10 %11 %13 → %15
        jsvalue dup %15 → %16 %17
        jsvalue cons_3 → %18
        sumArray run → f_ref1
        f_j2_j call f_ref1 %16 %18 → %19
        jsvalue cons_60 → %20
        jsvalue eq? %19 %20 → %21
        jsvalue assert %21
        jsvalue drop → %17
    )
)
```

## 6. A loop that mutates an object field — `counter`

```js
function makeCounter() {
    let c = {};
    c.count = 0;
    return c;
}

function incrementBy(c, n) {
    let i = 0;
    while (i < n) {
        c.count = c.count + 1;
        i = i + 1;
    }
    return c;
}

function run() {
    let c = makeCounter();
    c = incrementBy(c, 5);
    assert(c.count == 5);
}

run();
```

```
structure main
(
    run = λ
    (
        run run → f_ref1
        f__j call f_ref1 → %0
    )
)
structure makeCounter
(
    run = λ → out
    (
        jsvalue cons_obj → %0
        jsvalue cons_str "count" → %1
        jsvalue cons_0 → %2
        jsvalue dup %2 → %3 %4
        jsvalue set %0 %1 %3 → %5
        jsvalue dup %5 → %6 %7
        jsvalue move %6 → out
        jsvalue drop → %7
    )
)
structure incrementBy
(
    loop1 = λ %2 %1 %0 → packed
    (
        jsvalue dup %2 → %3 %4
        jsvalue dup %1 → %5 %6
        jsvalue lt? %3 %5 → %7
        jsvalue dup %7 → cmp16 cmp17
        jsvalue not cmp17 → cmp18
        incrementBy loopbody2 → ref19
        incrementBy loopexit3 → ref20
        incrementBy loopframe4 → ref21
        f_j3_j opt cmp16 ref19 → alt22
        f_j3_j opt cmp18 ref20 → alt23
        f_j3_j join alt22 alt23 ref21 → cont24
        f_j3_j call cont24 %4 %6 %0 → packed
    )
    loopbody2 = λ %2 %1 %0 → packed
    (
        jsvalue cons_str "count" → %8
        jsvalue dup %0 → %9 %10
        jsvalue cons_str "count" → %11
        jsvalue get %9 %11 → %12
        jsvalue cons_1 → %13
        jsvalue add %12 %13 → %14
        jsvalue dup %14 → %15 %16
        jsvalue set %10 %8 %15 → %17
        jsvalue dup %2 → %18 %19
        jsvalue cons_1 → %20
        jsvalue add %18 %20 → %21
        jsvalue drop → %19
        jsvalue copy %21 → %19
        jsvalue dup %19 → %22 %23
        incrementBy loop1 → self12
        f_j3_j call self12 %23 %1 %17 → packed
    )
    loopexit3 = λ %2 %1 %0 → arr11
    (
        jsvalue cons_arr → arr5
        jsvalue cons_0 → k6
        jsvalue set arr5 k6 %2 → arr7
        jsvalue cons_1 → k8
        jsvalue set arr7 k8 %1 → arr9
        jsvalue cons_2 → k10
        jsvalue set arr9 k10 %0 → arr11
    )
    loopframe4 = λ A B %2 %1 %0 → packed15
    (
        jsvalue dup %2 → %2_1 %2_2
        jsvalue dup %1 → %1_1 %1_2
        jsvalue dup %0 → %0_1 %0_2
        f_j3_j call A %2_1 %1_1 %0_1 → p113
        f_j3_j call B %2_2 %1_2 %0_2 → p214
        jsvalue join p113 p214 → packed15
    )
    run = λ %0 %1 → out
    (
        jsvalue cons_0 → %2
        incrementBy loop1 → ref25
        f_j3_j call ref25 %2 %1 %0 → packed26
        jsvalue dup packed26 → u27 u28
        jsvalue cons_0 → k29
        jsvalue get u27 k29 → %24
        jsvalue dup u28 → u30 u31
        jsvalue cons_1 → k32
        jsvalue get u30 k32 → %25
        jsvalue cons_2 → k33
        jsvalue get u31 k33 → %26
        jsvalue dup %26 → %27 %28
        jsvalue move %27 → out
        jsvalue drop → %24
        jsvalue drop → %25
        jsvalue drop → %28
    )
)
structure run
(
    run = λ
    (
        makeCounter run → f_ref1
        f__j call f_ref1 → %0
        jsvalue dup %0 → %1 %2
        jsvalue cons_5 → %3
        incrementBy run → f_ref2
        f_j2_j call f_ref2 %1 %3 → %4
        jsvalue drop → %2
        jsvalue copy %4 → %2
        jsvalue dup %2 → %5 %6
        jsvalue dup %6 → %7 %8
        jsvalue cons_str "count" → %9
        jsvalue get %7 %9 → %10
        jsvalue cons_5 → %11
        jsvalue eq? %10 %11 → %12
        jsvalue assert %12
        jsvalue drop → %8
    )
)
```