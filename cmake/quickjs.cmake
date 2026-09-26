include(FetchContent)

FetchContent_Declare(
    quickjs
    GIT_REPOSITORY https://github.com/bellard/quickjs.git
    GIT_TAG master
)

FetchContent_MakeAvailable(quickjs)

add_library(quickjs STATIC
    ${quickjs_SOURCE_DIR}/quickjs.c
    ${quickjs_SOURCE_DIR}/libregexp.c
    ${quickjs_SOURCE_DIR}/libunicode.c
    ${quickjs_SOURCE_DIR}/cutils.c
    ${quickjs_SOURCE_DIR}/dtoa.c
)

target_include_directories(quickjs PUBLIC
    ${quickjs_SOURCE_DIR}
)

target_compile_definitions(quickjs PRIVATE
    _GNU_SOURCE
    CONFIG_VERSION=\"2025-09-13\"
    HAVE_CLOSEFROM
)

target_link_libraries(quickjs PRIVATE m)

add_library(quickjs-libc STATIC
    ${quickjs_SOURCE_DIR}/quickjs-libc.c
)

target_include_directories(quickjs-libc PUBLIC
    ${quickjs_SOURCE_DIR}
)

target_compile_definitions(quickjs-libc PRIVATE
    _GNU_SOURCE
    CONFIG_VERSION=\"2025-09-13\"
    HAVE_CLOSEFROM
)

target_link_libraries(quickjs-libc PUBLIC quickjs m dl pthread)

add_executable(qjsc
    ${quickjs_SOURCE_DIR}/qjsc.c
)

target_link_libraries(qjsc PRIVATE quickjs-libc)

target_compile_definitions(qjsc PRIVATE
    _GNU_SOURCE
    CONFIG_VERSION=\"2025-09-13\"
    HAVE_CLOSEFROM
)

# Upstream's own JS runner/REPL -- lets a .js file be run directly
# (`qjs --stack-size n file.js`) without going through js2ct/ct2qjs at all.
# Useful for isolating whether an observed behavior comes from this
# project's compilers or from QuickJS itself (e.g. tmp/tail_call_stack_experiment.sh).
# qjs.c references a `qjsc_repl` byte array for its REPL mode; upstream
# generates it by using qjsc (once built) to compile its own repl.js.
set(QJS_REPL_C ${CMAKE_CURRENT_BINARY_DIR}/repl.c)

add_custom_command(
    OUTPUT ${QJS_REPL_C}
    COMMAND qjsc -s -c -o ${QJS_REPL_C} -m ${quickjs_SOURCE_DIR}/repl.js
    DEPENDS qjsc ${quickjs_SOURCE_DIR}/repl.js
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(qjs
    ${quickjs_SOURCE_DIR}/qjs.c
    ${QJS_REPL_C}
)

target_link_libraries(qjs PRIVATE quickjs-libc)

target_compile_definitions(qjs PRIVATE
    _GNU_SOURCE
    CONFIG_VERSION=\"2025-09-13\"
    HAVE_CLOSEFROM
)
