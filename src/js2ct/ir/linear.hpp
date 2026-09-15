#pragma once

#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <variant>
#include <vector>

#include "../sema/types.hpp"

namespace qthu::js2ct::lin {
    struct value {
        uint32_t id;
    };

    inline bool operator<(const value &lhs, const value &rhs) {
        return lhs.id < rhs.id;
    }

    // std::monostate marks the "undefined" constant -- distinct from any
    // uint64_t/bool value, and needs no payload of its own.
    using constant = std::variant<uint64_t, bool, std::monostate>;
    using argument = std::variant<constant, value>;

    struct instr;

    struct cons_data {
        constant c;
        value target;
    };

    struct str_cons_data {
        std::string str;
        value target;
    };

    struct get_data {
        argument obj;
        argument key;
        value target;
    };

    struct cons_obj_data {
        value target;
    };

    struct cons_arr_data {
        value target;
    };

    struct set_data {
        argument obj;
        argument key;
        argument val;
        value target;
    };

    struct unary_data {
        op_kind op;
        argument arg1;
        value target;
    };

    struct binary_data {
        op_kind op;
        argument arg1;
        argument arg2;
        value target;
    };

    struct copy_data {
        argument arg1;
        value target;
    };

    struct dup_data {
        argument arg1;
        value first;
        value second;
    };

    struct if_data {
        argument cond;
        std::vector<instr> then_body;
        std::vector<instr> else_body;
        std::vector<value> params;
        bool exhaustively_returns = false;
        std::vector<value> then_outputs;
        std::vector<value> else_outputs;
        std::vector<value> outputs;
    };

    struct loop_data {
        std::vector<instr> cond_body;
        argument cond;
        std::vector<value> dispatch_args;
        std::vector<instr> body;
        std::vector<value> params;
        std::vector<value> next_params;
        std::vector<value> outputs;
    };

    struct drop_data {
        value target;
    };

    struct call_data {
        sema::function_id callee;
        std::vector<argument> args;
        value target;
    };

    struct ret_data {
        std::optional<argument> arg;
    };

    struct brk_data {
    };

    struct cont_data {
    };

    struct assert_data {
        argument arg;
    };

    struct instr {
        using data_type = std::variant<
            cons_data,
            str_cons_data,
            unary_data,
            binary_data,
            copy_data,
            dup_data,
            drop_data,
            if_data,
            loop_data,
            call_data,
            get_data,
            cons_obj_data,
            cons_arr_data,
            set_data,
            ret_data,
            assert_data,
            brk_data,
            cont_data>;
        data_type data;

        void for_each_use(auto &&f) {
            auto visit_operand = [ & ](argument &o) {
                if (auto *v = std::get_if<value>(&o))
                    f(*v);
            };

            std::visit([ & ](auto &&d) {
                using T = std::decay_t<decltype( d )>;
                if constexpr (std::is_same_v<T, unary_data>)
                    visit_operand(d.arg1);
                else if constexpr (std::is_same_v<T, binary_data>) {
                    visit_operand(d.arg1);
                    visit_operand(d.arg2);
                } else if constexpr (std::is_same_v<T, copy_data>)
                    visit_operand(d.arg1);
                else if constexpr (std::is_same_v<T, if_data>)
                    visit_operand(d.cond);
                else if constexpr (std::is_same_v<T, call_data>)
                    for (auto &a: d.args)
                        visit_operand(a);
                else if constexpr (std::is_same_v<T, get_data>) {
                    visit_operand(d.obj);
                    visit_operand(d.key);
                } else if constexpr (std::is_same_v<T, set_data>) {
                    visit_operand(d.obj);
                    visit_operand(d.key);
                    visit_operand(d.val);
                } else if constexpr (std::is_same_v<T, ret_data>) {
                    if (d.arg)
                        visit_operand(*d.arg);
                } else if constexpr (std::is_same_v<T, assert_data>)
                    visit_operand(d.arg);
            }, data);
        }

        void set_target(value new_target) {
            std::visit([ & ](auto &&d) {
                using T = std::decay_t<decltype( d )>;
                if constexpr (requires { d.target; })
                    d.target = new_target;
            }, data);
        }

        std::optional<value> get_target() {
            return std::visit([](auto &&d) -> std::optional<value> {
                using T = std::decay_t<decltype( d )>;
                if constexpr (requires { d.target; })
                    return d.target;
                else
                    return std::nullopt;
            }, data);
        }
    };

    struct function {
        sema::function_id name;
        std::vector<value> params;
        std::vector<instr> body;
    };

    struct program {
        std::vector<function> functions;

        function &get_script() {
            assert(!functions.empty());
            return functions[0];
        }
    };
}
