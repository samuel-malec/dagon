#pragma once

#include <map>
#include <string>
#include <vector>

/**
 * hicthu is literally the same thing as locthu except for one thing,
 * and that is functions. Hicthu uses explicit discrete return values,
 * whereas locthu uses only one return value, where all return values from hicthu
 * are packed into one array.
 */
namespace qthu::js2ct::hicthu {
    using name = std::string;

    struct insn {
        name structure;
        name operation;
        std::vector<name> in;
        std::vector<name> out;
    };

    struct function {
        std::vector<name> in;
        std::vector<name> out;
        std::vector<insn> body;
    };

    struct structure {
        name id;
        std::map<name, function> functions;
    };

    struct module {
        std::vector<structure> structures;
        std::vector<std::string> strings;
    };
} // namespace qthu::js2ct::hicthu
