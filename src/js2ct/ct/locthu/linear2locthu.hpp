#pragma once

#include "../cthu.hpp"
#include "../hicthu/hicthu.hpp"
#include "../hicthu/hicthu2locthu.hpp"
#include "linear2hicthu.hpp"
#include "../../lin/linear.hpp"
#include "../../sema/analysis.hpp"

namespace qthu::js2ct::cthu {
    // The full linear -> hicthu -> locthu path. A .ct file is locthu, so this is
    // what the driver writes out; hicthu exists in between because that is where
    // an `if` or a loop can still hand back its live bindings as separate values
    // instead of as one packed array.
    inline cthu::module lower_to_locthu(lin::program &prog, sema::analysis_result &sema) {
        hicthu::hict_lowerer to_hicthu{sema};
        loct_lowerer to_locthu{};
        return to_locthu.lower(to_hicthu.lower(prog));
    }
}
