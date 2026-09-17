cmake_minimum_required(VERSION 3.10)
project(qthu)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# emulator of an assembler with qjs-like opcodes
add_executable(qasm)
include(cmake/quickjs.cmake)
target_sources(qasm
    PRIVATE
    src/asm/qasm.cpp
    src/asm/asm.cpp
    src/asm/instruction.cpp
    src/asm/directive.cpp
)

target_link_libraries(qasm
    PRIVATE 
    quickjs
)

# qjs bytecode runner
add_executable(bcrun)
target_sources(bcrun
    PRIVATE
    src/bcrun.c
)

target_link_libraries(bcrun
    PRIVATE
    quickjs-libc
)

# compiler from cthu to qjs bytecode
add_executable(ct2qjs)
target_sources(ct2qjs
    PRIVATE
        src/ct2qjs/main.cpp
        src/ct2qjs/frontend/lexer.cpp
        src/ct2qjs/frontend/reader.cpp
        src/ct2qjs/codegen/codegen.cpp
    src/asm/asm.cpp
    src/asm/instruction.cpp
    src/asm/directive.cpp
)

target_include_directories(ct2qjs
    PRIVATE
    ${CMAKE_SOURCE_DIR}/bricks
)

target_link_libraries(ct2qjs
    PRIVATE
    quickjs-libc
)

# compiler from javascript to cthu
add_executable(js2ct)

target_sources(js2ct
    PRIVATE
    src/js2ct/main.cpp
    src/js2ct/frontend/lexer.cpp
    src/js2ct/frontend/parser.cpp
    src/js2ct/printer/pretty_printer.cpp
)

# Browser build: js2ct + ct2qjs + bytecode execution, all in one WASM module,
# for an in-browser pipeline explorer (see src/wasm/api.cpp). Only built
# under an Emscripten toolchain (emcmake), never as part of a normal build.
if (EMSCRIPTEN)
    file(READ ${CMAKE_SOURCE_DIR}/src/cthu_core/js_dial/prelude.ct QTHU_WASM_PRELUDE_CT)
    file(READ ${CMAKE_SOURCE_DIR}/src/cthu_core/js_dial/builtins.ct QTHU_WASM_BUILTINS_CT)
    file(WRITE ${CMAKE_BINARY_DIR}/generated/wasm_embedded_prelude.hpp
        "#pragma once\n"
        "// Auto-generated from src/cthu_core/js_dial/{prelude,builtins}.ct -- see CMakeLists.txt.\n"
        "namespace qthu::wasm {\n"
        "inline const char *PRELUDE_CT = R\"QTHU_CT__(${QTHU_WASM_PRELUDE_CT})QTHU_CT__\";\n"
        "inline const char *BUILTINS_CT = R\"QTHU_CT__(${QTHU_WASM_BUILTINS_CT})QTHU_CT__\";\n"
        "}\n"
    )

    add_executable(qthu-wasm
        src/wasm/api.cpp
        src/js2ct/frontend/lexer.cpp
        src/js2ct/frontend/parser.cpp
        src/js2ct/printer/pretty_printer.cpp
            src/ct2qjs/frontend/lexer.cpp
            src/ct2qjs/frontend/reader.cpp
            src/ct2qjs/codegen/codegen.cpp
        src/asm/asm.cpp
        src/asm/instruction.cpp
        src/asm/directive.cpp
    )

    target_include_directories(qthu-wasm
        PRIVATE
        ${CMAKE_SOURCE_DIR}/bricks
        ${CMAKE_BINARY_DIR}/generated
    )

    target_link_libraries(qthu-wasm
        PRIVATE
        quickjs
    )

    target_compile_options(qthu-wasm PRIVATE "-fwasm-exceptions")
    target_link_options(qthu-wasm PRIVATE
        "-lembind" "-sMODULARIZE=1" "-sEXPORT_ES6=1"
        "-sENVIRONMENT=web" "-sALLOW_MEMORY_GROWTH=1"
        # Default Emscripten stack (64KB) is too small for this pipeline's
        # recursive-descent lowering across two chained compilers (js2ct's
        # HIR/LIN passes, then ct2qjs's own recursive codegen) -- manifests
        # as a generic "memory access out of bounds" trap (WASM has no
        # guard-page stack-overflow detection the way native does), not a
        # clean stack-overflow error. Confirmed via native AddressSanitizer:
        # the identical logic runs clean natively, so this isn't a real
        # memory-safety bug, just a too-small WASM stack.
        "-sSTACK_SIZE=8MB"
        "-fwasm-exceptions"
    )
endif()
