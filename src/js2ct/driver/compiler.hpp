#pragma once

#include "config.hpp"
#include "../printer/pretty_printer.hpp"
#include "../../common/file.hpp"
#include "../frontend/token.hpp"
#include "../frontend/parser.hpp"
#include "../hir/hir.hpp"
#include "../ct/cthu.hpp"
#include "../hir/ast2hir.hpp"
#include "../lin/hir2linear.hpp"
#include "../ct/locthu/hicthu2locthu.hpp"
#include "../ct/hicthu/linear2hicthu.hpp"
#include "../sema/analysis.hpp"
#include "../../common/progress_reporter.hpp"

namespace qthu::js2ct {
    struct compiler {
        void run(const config &conf) {
            std::string in_name = conf.file_in;
            std::string out_name = conf.file_out;
            source_ptr doc = std::make_shared<source_file>(in_name, read_file(in_name));
            print::pretty_printer printer{};
            progress_reporter reporter{};

            ast::program ast;
            {
                reporter.time("parsing");
                parser p{doc};
                ast = p.parse();
            }

            if (conf.emit_ast)
                printer.print_ast(std::cout, ast);

            if (conf.parse_only)
                return;

            sema::analyzer analyzer;
            sema::analysis_result semantics;
            {
                reporter.time("semantic analysis");
                semantics = analyzer.run(ast);
            }

            hir::lowerer hir_lowerer{semantics};
            hir::module hir;
            {
                reporter.time("hir lowering");
                hir = hir_lowerer.lower(ast);
            }
            if (conf.emit_hir)
                printer.print_hir(std::cout, hir, semantics);

            lin::lowerer lin_lowerer{semantics};
            lin::program linear;
            {
                reporter.time("lin lowering");
                linear = lin_lowerer.lower(hir);
            }
            if (conf.emit_lin)
                printer.print_lin_program(std::cout, linear);

            cthu::module hict;
            {
                reporter.time("lowering to hicthu");
                hicthu::hict_lowerer hicthu_lowerer{semantics};
                hict = hicthu_lowerer.lower(linear);
            }
            if (conf.emit_hicthu)
                printer.print_cthu(std::cout, hict);

            cthu::module ct;
            {
                reporter.time("lowering to locthu");
                cthu::loct_lowerer locthu_lowerer{};
                ct = locthu_lowerer.lower(hict);
            }
            std::ofstream out(conf.file_out);
            if (!out.is_open())
                throw std::runtime_error("Couldn't open file at: " + conf.file_out);

            printer.print_cthu(out, ct);
        }
    };
}
