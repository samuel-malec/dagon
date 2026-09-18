#include <emscripten/bind.h>
#include <sstream>
#include <string>

#include "quickjs.h"

#include "wasm_embedded_prelude.hpp"

#include "../js2ct/frontend/parser.hpp"
#include "../js2ct/sema/analysis.hpp"
#include "../js2ct/hir/ast2hir.hpp"
#include "../js2ct/lin/hir2linear.hpp"
#include "../js2ct/printer/pretty_printer.hpp"

#include "../ct2qjs/frontend/reader.hpp"
#include "../ct2qjs/ir/ir.hpp"
#include "../ct2qjs/codegen/codegen.hpp"
#include "../asm/asmbuilder.hpp"
#include "../js2ct/ct/hicthu/linear2hicthu.hpp"
#include "../js2ct/ct/locthu/hicthu2locthu.hpp"

namespace qthu::wasm {
    struct pipeline_result {
        std::string ast, hir, lir, locthu, hicthu, bytecode;
        std::string run_output;
        std::string stage;
        std::string error;
    };

    void parse_ct_into(const std::string &name, const std::string &text, ct2qjs::symtab &st) {
        auto doc = std::make_shared<ct2qjs::source_file>(name, text);
        ct2qjs::reader r{doc, st};
        ct2qjs::diag err = r.parse();
        if (!err)
            return;

        brq::string_builder b;
        err->print(b);
        throw std::runtime_error(std::string(b.data()));
    }

    pipeline_result compile_and_run(const std::string &source) {
        pipeline_result r;
        js2ct::print::pretty_printer printer{};

        try {
            // ---- js2ct: JS source -> AST -> HIR -> LIN -> HCthu -> LCthu ----
            auto doc = std::make_shared<js2ct::source_file>("<input>", source);
            js2ct::parser p{doc};
            auto ast = p.parse();
            {
                std::ostringstream oss;
                printer.print_ast(oss, ast);
                r.ast = oss.str();
            }
            r.stage = "js2ct: parse";

            js2ct::sema::analyzer analyzer;
            auto semantics = analyzer.run(ast);
            r.stage = "js2ct: sema";

            js2ct::hir::lowerer hir_lowerer{semantics};
            js2ct::hir::module hir_mod = hir_lowerer.lower(ast);
            {
                std::ostringstream oss;
                printer.print_hir(oss, hir_mod, semantics);
                r.hir = oss.str();
            }
            r.stage = "js2ct: hir";

            js2ct::lin::lowerer lin_lowerer{semantics};
            js2ct::lin::program lir = lin_lowerer.lower(hir_mod);
            {
                std::ostringstream oss;
                printer.print_lin_program(oss, lir);
                r.lir = oss.str();
            }
            r.stage = "js2ct: lir";

            js2ct::hicthu::hict_lowerer hicthu_lowerer{semantics};
            js2ct::cthu::module hicthu = hicthu_lowerer.lower(lir);
            {
                std::ostringstream oss;
                printer.print_cthu(oss, hicthu);
                r.hicthu = oss.str();
            }
            r.stage = "js2ct: hicthu";

            js2ct::cthu::loct_lowerer locthu_lowerer{};
            js2ct::cthu::module locthu = locthu_lowerer.lower(hicthu);
            {
                std::ostringstream oss;
                printer.print_cthu(oss, locthu);
                r.locthu = oss.str();
            }
            r.stage = "js2ct: locthu";

            // ---- ct2qjs: Cthulhu -> QuickJS bytecode ----
            ct2qjs::symtab st;
            parse_ct_into("prelude.ct", PRELUDE_CT, st);
            parse_ct_into("builtins.ct", BUILTINS_CT, st);
            parse_ct_into("<generated>", r.locthu, st);
            r.stage = "ct2qjs: parse";

            ct2qjs::program ir_prog{st};
            ir_prog.lower_to_ir();
            r.stage = "ct2qjs: lower_to_ir";

            as::asmbuilder builder{};
            ct2qjs::codegen cg{ir_prog, builder};
            bc::program bc_prog = cg.lower_to_bc();
            r.bytecode = builder.print_asm();
            r.stage = "ct2qjs: codegen";

            std::vector<uint8_t> bytes = bc_prog.to_bytes();

            // ---- execute the bytecode against a real QuickJS runtime ----
            JSRuntime *rt = JS_NewRuntime();
            JSContext *ctx = JS_NewContext(rt);

            JSValue obj = JS_ReadObject(ctx, bytes.data(), bytes.size(), JS_READ_OBJ_BYTECODE);
            if (JS_IsException(obj)) {
                // Matches bcrun.c: a failed JS_ReadObject's exception value
                // isn't freed separately either.
                JSValue exc = JS_GetException(ctx);
                const char *msg = JS_ToCString(ctx, exc);
                r.run_output = std::string("failed to load bytecode: ") + (msg ? msg : "?");
                JS_FreeCString(ctx, msg);
                JS_FreeValue(ctx, exc);
            } else {
                // JS_EvalFunction takes ownership of `obj` and frees it
                // internally (same contract bcrun.c relies on) -- freeing it
                // again here would be a use-after-free, caught directly with
                // AddressSanitizer while debugging the WASM build's crash.
                JSValue val = JS_EvalFunction(ctx, obj);
                if (JS_IsException(val)) {
                    JSValue exc = JS_GetException(ctx);
                    const char *msg = JS_ToCString(ctx, exc);
                    r.run_output = std::string("Runtime exception: ") + (msg ? msg : "?");
                    JS_FreeCString(ctx, msg);
                    JS_FreeValue(ctx, exc);
                } else {
                    JSValue str_val = JS_ToString(ctx, val);
                    const char *s = JS_ToCString(ctx, str_val);
                    r.run_output = std::string("Returned value: ") + (s ? s : "undefined");
                    JS_FreeCString(ctx, s);
                    JS_FreeValue(ctx, str_val);
                }
                JS_FreeValue(ctx, val);
            }
            JS_FreeContext(ctx);
            JS_FreeRuntime(rt);

            r.stage = "done";
        } catch (const std::exception &e) {
            r.error = e.what();
        }

        return r;
    }
}

EMSCRIPTEN_BINDINGS (qthu) {
    emscripten::value_object<qthu::wasm::pipeline_result>("PipelineResult")
            .field("ast", &qthu::wasm::pipeline_result::ast)
            .field("hir", &qthu::wasm::pipeline_result::hir)
            .field("lir", &qthu::wasm::pipeline_result::lir)
            .field("locthu", &qthu::wasm::pipeline_result::locthu)
            .field("hicthu", &qthu::wasm::pipeline_result::hicthu)
            .field("bytecode", &qthu::wasm::pipeline_result::bytecode)
            .field("runOutput", &qthu::wasm::pipeline_result::run_output)
            .field("stage", &qthu::wasm::pipeline_result::stage)
            .field("error", &qthu::wasm::pipeline_result::error);
    emscripten::function("compile", &qthu::wasm::compile_and_run);
}
