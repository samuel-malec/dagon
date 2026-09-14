#!/usr/bin/env bash
#
# Tests ct2qjs's own lexer and reader (the .ct-format side of the pipeline,
# as opposed to test/run_js2ct_frontend.sh which tests js2ct's JS-side
# lexer/parser) in isolation from what a fixture's *runtime behavior* is.
#
# Unlike js2ct, ct2qjs has no --parse-only flag: it always runs the full
# parse -> lower_to_ir -> codegen -> write-bytecode pipeline. So a "good"
# fixture here must be a complete, syntactically-minimal but fully working
# .ct program (compiles to bytecode; nothing asserts its runtime output --
# that's test/run.sh's job for the full test/ct2qjs/*.ct suite). A "bad"
# fixture just needs to be rejected somewhere in that pipeline -- reader,
# lexer, or a later stage -- as long as it's rejected *cleanly*.
#
# Covers:
#   test/ct2qjs/lexer/good/*.ct   -- must compile successfully
#   test/ct2qjs/lexer/bad/*.ct    -- must be rejected cleanly (lexical errors)
#   test/ct2qjs/parser/good/*.ct  -- must compile successfully
#   test/ct2qjs/parser/bad/*.ct   -- must be rejected cleanly (grammar errors)
#
# "Cleanly" means a thrown exception (nonzero exit), not a crash (killed by
# a signal) and not a hang -- see reader.cpp's `peek()`: truncated/malformed
# input used to spin forever instead of ever returning, since a loop only
# guarded "haven't found a token yet", not "and there's still input left to
# find one in". A fixture that hangs the compiler is exactly as much its own
# bug as one that crashes it, so every fixture here runs under `timeout`,
# and a timeout is its own distinct, clearly-labeled failure mode (not
# folded into "crashed"), which is genuinely worse: it doesn't even fail
# the surrounding shell script, just hangs it, unless it's explicitly
# guarded like this everywhere the compiler is invoked from a script.
#
# Usage: test/run_ct2qjs_frontend.sh
# Override the binary with an env var if it's not in one of the usual
# build directories: CT2QJS=/path/to/ct2qjs test/run_ct2qjs_frontend.sh

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PRELUDE_DIR="$REPO_ROOT/src/cthu_core/js_dial"
TIMEOUT_SECS=10

find_bin()
{
    local name="$1"
    local override="$2"

    if [[ -n "$override" ]]; then
        if [[ -x "$override" ]]; then
            echo "$override"
            return 0
        fi
        echo "error: $name override '$override' is not an executable file" >&2
        return 1
    fi

    local candidate
    for candidate in \
        "$REPO_ROOT/cmake-build-debug/$name" \
        "$REPO_ROOT/out/build/qthu/$name" \
        "$REPO_ROOT/build/$name"
    do
        if [[ -x "$candidate" ]]; then
            echo "$candidate"
            return 0
        fi
    done

    return 1
}

CT2QJS_BIN="$(find_bin ct2qjs "${CT2QJS:-}")" || {
    echo "error: could not find a built 'ct2qjs' binary. Build it first, or point to it with CT2QJS=/path/to/ct2qjs" >&2
    exit 2
}

pass=0
fail=0
fail_names=()

# check_fixture <file> <expect: good|bad>
check_fixture()
{
    local file="$1"
    local expect="$2"
    local name
    name="$(basename "$file")"

    local out
    out="$(timeout "$TIMEOUT_SECS" "$CT2QJS_BIN" "$file" -p "$PRELUDE_DIR" -o /dev/null 2>&1)"
    local status=$?

    if (( status == 124 || status == 137 )); then
        echo "FAIL  $name  (hung -- killed after ${TIMEOUT_SECS}s)"
        fail=$((fail + 1))
        fail_names+=("$name")
        return
    fi

    if (( status >= 129 && status <= 192 )); then
        echo "FAIL  $name  (crashed, exit=$status -- likely killed by a signal)"
        echo "$out" | sed 's/^/        /'
        fail=$((fail + 1))
        fail_names+=("$name")
        return
    fi

    if [[ "$expect" == "good" ]]; then
        if (( status == 0 )); then
            echo "PASS  $name"
            pass=$((pass + 1))
        else
            echo "FAIL  $name  (expected to compile, but ct2qjs rejected it)"
            echo "$out" | sed 's/^/        /'
            fail=$((fail + 1))
            fail_names+=("$name")
        fi
    else
        if (( status != 0 )); then
            echo "PASS  $name"
            pass=$((pass + 1))
        else
            echo "FAIL  $name  (expected to be rejected, but ct2qjs compiled it)"
            fail=$((fail + 1))
            fail_names+=("$name")
        fi
    fi
}

shopt -s nullglob

echo "== lexer/good =="
for f in "$REPO_ROOT"/test/ct2qjs/lexer/good/*.ct; do
    check_fixture "$f" good
done

echo "== lexer/bad =="
for f in "$REPO_ROOT"/test/ct2qjs/lexer/bad/*.ct; do
    check_fixture "$f" bad
done

echo "== parser/good =="
for f in "$REPO_ROOT"/test/ct2qjs/parser/good/*.ct; do
    check_fixture "$f" good
done

echo "== parser/bad =="
for f in "$REPO_ROOT"/test/ct2qjs/parser/bad/*.ct; do
    check_fixture "$f" bad
done

shopt -u nullglob

echo
echo "$pass passed, $fail failed"

if [[ $fail -gt 0 ]]; then
    echo "failed: ${fail_names[*]}"
    exit 1
fi

exit 0
