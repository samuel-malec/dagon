#pragma once

#include "builder.hpp"
#include "../../lin/linear.hpp"
#include "../../sema/analysis.hpp"

namespace qthu::js2ct::hicthu {
    struct hict_lowerer {
        sema::analysis_result &sema;

        cthu::module lower(lin::program &prog) {
            cthu::module mod{};
            for (int i = 0; i < prog.functions.size(); ++i) {
                std::string struct_name = i == 0
                                              ? "__toplevel__"
                                              : sema.function_name(prog.functions[i].name);
                cthu::structure_builder sb{struct_name, prog.functions[i], sema, mod.strings};
                mod.structures.push_back(std::move(sb.lower()));
            }
            return mod;
        }
    };
}
