#pragma once

#include <string>
#include <vector>

#include "../cthu.hpp"
#include "hicthu.hpp"

namespace qthu::js2ct::cthu {
    struct loct_lowerer {
        uint32_t next_tmp = 1;

        std::string fresh(const std::string &prefix) {
            return prefix + std::to_string(next_tmp++);
        }

        static bool is_signature_op(const hicthu::insn &i) {
            return (i.operation == "call" || i.operation == "opt" || i.operation == "join")
                   && i.structure.starts_with("f_");
        }

        static std::string collapse_signature(const std::string &sig) {
            auto pos = sig.rfind('_');
            return sig.substr(0, pos + 1) + "j";
        }

        // arr = [] arr[k] = values[k], for each k. Returns the name holding the array.
        std::string pack(std::vector<insn> &body, const std::vector<name> &values) {
            std::string cur = fresh("pk");
            body.push_back(insn{"jsvalue", "cons_arr", {}, {cur}});

            for (size_t k = 0; k < values.size(); ++k) {
                std::string key = fresh("pi");
                body.push_back(insn{"jsvalue", "cons_" + std::to_string(k), {}, {key}});

                std::string next = fresh("pk");
                body.push_back(insn{"jsvalue", "set", {cur, key, values[k]}, {next}});
                cur = next;
            }

            return cur;
        }

        void unpack(std::vector<insn> &body, const std::string &packed, const std::vector<name> &targets) {
            std::string cur = packed;

            for (size_t k = 0; k < targets.size(); ++k) {
                std::string src = cur;

                if (k + 1 < targets.size()) {
                    std::string a = fresh("pu");
                    std::string b = fresh("pu");
                    body.push_back(insn{"jsvalue", "dup", {cur}, {a, b}});
                    src = a;
                    cur = b;
                }

                std::string key = fresh("pi");
                body.push_back(insn{"jsvalue", "cons_" + std::to_string(k), {}, {key}});
                body.push_back(insn{"jsvalue", "get", {src, key}, {targets[k]}});
            }
        }

        function lower_function(const hicthu::function &hi) {
            function lo{};
            lo.in = hi.in;

            const bool forward_tail = hi.out.size() > 1
                                      && !hi.body.empty()
                                      && hi.body.back().operation == "call"
                                      && hi.body.back().out == hi.out;

            for (size_t idx = 0; idx < hi.body.size(); ++idx) {
                const auto &i = hi.body[idx];
                insn lowered{
                    is_signature_op(i) ? collapse_signature(i.structure) : i.structure,
                    i.operation,
                    i.in,
                    i.out,
                };

                if (i.operation == "call" && i.out.size() > 1) {
                    std::string packed = fresh("pk");
                    lowered.out = {packed};
                    lo.body.push_back(std::move(lowered));

                    if (forward_tail && idx + 1 == hi.body.size()) {
                        lo.out = {packed};
                        return lo;
                    }

                    unpack(lo.body, packed, i.out);
                    continue;
                }

                lo.body.push_back(std::move(lowered));
            }

            if (hi.out.size() > 1)
                lo.out = {pack(lo.body, hi.out)};
            else
                lo.out = hi.out;

            return lo;
        }

        structure lower_structure(const hicthu::structure &hi) {
            structure lo{.id = hi.id};
            for (const auto &[fname, hi_fn]: hi.functions)
                lo.functions[fname] = lower_function(hi_fn);
            return lo;
        }

        cthu::module lower(const hicthu::module &hi) {
            cthu::module lo{};
            lo.strings = hi.strings;
            for (const auto &s: hi.structures)
                lo.structures.push_back(lower_structure(s));
            return lo;
        }
    };
}
