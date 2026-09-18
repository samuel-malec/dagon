#pragma once

#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../../lin/linear.hpp"
#include "../../printer/pretty_printer.hpp"
#include "../../sema/analysis.hpp"

namespace qthu::js2ct::cthu {
    struct structure_builder {
        std::string struct_name;
        lin::function &fn;
        sema::analysis_result &sema;
        std::vector<std::string> &strings; // shared module-wide string-constant pool
        structure *curr_struct = nullptr;
        uint32_t next_val = 1;

        size_t intern_string(const std::string &s) const {
            for (size_t i = 0; i < strings.size(); ++i)
                if (strings[i] == s)
                    return i;
            strings.push_back(s);
            return strings.size() - 1;
        }

        static std::vector<std::string> vals2str(const std::vector<lin::value> &vals) {
            std::vector<std::string> res{};
            print::pretty_printer pp{};
            for (auto &val: vals) {
                std::ostringstream os;
                pp.print_lin_value(os, val);
                res.push_back(std::move(os.str()));
            }
            return res;
        }

        static std::vector<std::string> args2str(const std::vector<lin::argument> &args) {
            std::vector<std::string> res{};
            print::pretty_printer pp{};
            for (auto &arg: args) {
                std::ostringstream os;
                pp.print_lin_argument(os, arg);
                res.push_back(std::move(os.str()));
            }

            return res;
        }

        static void emit(function &fn,
                         std::string structure,
                         std::string op,
                         std::vector<std::string> in,
                         std::vector<std::string> out) {
            fn.body.push_back(insn{structure, op, std::move(in), std::move(out)});
        }

        void emit(function &fn,
                  std::string structure,
                  std::string op,
                  std::vector<lin::argument> in,
                  std::vector<lin::argument> out) {
            fn.body.push_back(insn{structure, op, std::move(args2str(in)), std::move(args2str(out))});
        }

        static std::string op_to_str(op_kind op) {
            switch (op) {
                case ADD: return "add";
                case SUB: return "sub";
                case MUL: return "mul";
                case DIV: return "div";
                case MOD: return "rem";
                case EQ: return "eq?";
                case NEQ: return "ne?";
                case LT: return "lt?";
                case LEQ: return "le?";
                case GT: return "gt?";
                case GEQ: return "ge?";
                case SEQ: return "seq?";
                case SNEQ: return "sne?";
                case SHL: return "shl";
                case SHR: return "shr";
                case NOT: return "not";
                case BAND: return "band";
                case BOR: return "bor";
                case BXOR: return "bxor";
                case BNOT: return "bnot";
                case AND:
                    break;
                case OR:
                    break;
            }

            throw std::runtime_error("op_to_str: unhandled op_kind");
        }

        std::string unary_op_to_str(op_kind op) {
            if (op == SUB)
                return "neg";
            return op_to_str(op);
        }

        std::string compact_run(char c, size_t k) {
            if (k == 0)
                return "";
            if (k == 1)
                return std::string(1, c);
            return std::string(1, c) + std::to_string(k);
        }

        // f_<inputs>_<outputs>, e.g. f_j3_j2 takes three jsvalues and returns two.
        // locthu collapses every one of these to a single-output signature.
        std::string call_signature_name(size_t n_in, size_t n_out) {
            return "f_" + compact_run('j', n_in) + "_" + compact_run('j', n_out);
        }

        std::string fresh_val(std::string prefix) {
            return prefix + std::to_string(next_val++);
        }

        std::vector<std::string> fresh_vals(std::string prefix, size_t n) {
            std::vector<std::string> res;
            for (size_t k = 0; k < n; ++k)
                res.push_back(fresh_val(prefix));
            return res;
        }

        // The frame is the operand `join` falls back on when both alternatives
        // survive: it runs both and merges them result-by-result.
        function create_frame(const std::vector<std::string> &params, const std::string &fsig, size_t n_out) {
            function res{};
            std::vector<std::string> param_names{"A", "B"};
            for (auto &s: params)
                param_names.push_back(s);
            res.in = param_names;

            std::vector<std::string> call_a{"A"};
            std::vector<std::string> call_b{"B"};

            for (size_t i = 2; i < res.in.size(); ++i) {
                std::string fst = res.in[i] + "_1";
                std::string snd = res.in[i] + "_2";
                call_a.push_back(fst);
                call_b.push_back(snd);
                emit(res, "jsvalue", "dup", {res.in[i]}, {fst, snd});
            }

            std::vector<std::string> outs_a = fresh_vals("fa", n_out);
            std::vector<std::string> outs_b = fresh_vals("fb", n_out);
            std::vector<std::string> outs = fresh_vals("fo", n_out);

            emit(res, fsig, "call", call_a, outs_a);
            emit(res, fsig, "call", call_b, outs_b);

            for (size_t k = 0; k < n_out; ++k)
                emit(res, "jsvalue", "join", {outs_a[k], outs_b[k]}, {outs[k]});

            res.out = outs;
            return res;
        }

        void lower_fn(std::string name, std::vector<lin::instr> &ins, std::vector<insn> extra = {}) {
            function curr_fn{};

            for (auto &i: ins) {
                std::visit(overloaded{
                               [ & ](lin::cons_data &cd) {
                                   if (auto *int_val = std::get_if<uint64_t>(&cd.c))
                                       emit(curr_fn, "jsvalue", "cons_" + std::to_string(*int_val), {}, {cd.target});
                                   else if (auto *bool_val = std::get_if<bool>(&cd.c))
                                       emit(curr_fn, "jsvalue", "cons_" + std::string(*bool_val ? "true" : "false"),
                                            {}, {cd.target});
                                   else
                                       emit(curr_fn, "jsvalue", "cons_undef", {}, {cd.target});
                               },
                               [ & ](lin::str_cons_data &sd) {
                                   size_t idx = intern_string(sd.str);
                                   emit(curr_fn, "jsvalue", "cons_str_" + std::to_string(idx), {}, {sd.target});
                               },
                               [ & ](lin::get_data &gd) {
                                   emit(curr_fn, "jsvalue", "get", {gd.obj, gd.key}, {gd.target});
                               },
                               [ & ](lin::cons_obj_data &cod) {
                                   emit(curr_fn, "jsvalue", "cons_obj", {}, {cod.target});
                               },
                               [ & ](lin::cons_arr_data &cad) {
                                   emit(curr_fn, "jsvalue", "cons_arr", {}, {cad.target});
                               },
                               [ & ](lin::set_data &sd) {
                                   emit(curr_fn, "jsvalue", "set", {sd.obj, sd.key, sd.val}, {sd.target});
                               },
                               [ & ](lin::unary_data &u) {
                                   emit(curr_fn, "jsvalue", unary_op_to_str(u.op), {u.arg1}, {u.target});
                               },
                               [ & ](lin::binary_data &b) {
                                   emit(curr_fn, "jsvalue", op_to_str(b.op), {b.arg1, b.arg2}, {b.target});
                               },
                               [ & ](lin::copy_data &c) {
                                   emit(curr_fn, "jsvalue", "copy", {c.arg1}, {c.target});
                               },
                               [ & ](lin::dup_data &dd) {
                                   emit(curr_fn, "jsvalue", "dup", {dd.arg1}, {dd.first, dd.second});
                               },
                               [ & ](lin::drop_data &dr) {
                                   emit(curr_fn, "jsvalue", "drop", {}, {dr.target});
                               },
                               [ & ](lin::if_data &id) {
                                   std::string cmp1 = fresh_val("cmp");
                                   std::string cmp2 = fresh_val("cmp");
                                   std::string cmp3 = fresh_val("cmp");

                                   emit(curr_fn, "jsvalue", "dup", args2str({id.cond}), {cmp1, cmp2});
                                   emit(curr_fn, "jsvalue", "not", {cmp2}, {cmp3});

                                   std::string then_name = fresh_val("then");
                                   std::string else_name = fresh_val("else");
                                   std::string frame_name = fresh_val("frame");

                                   std::vector<std::string> params = vals2str(id.params);
                                   std::vector<std::string> outs = vals2str(id.outputs);

                                   // An exhaustive if returns out of the enclosing function, so both
                                   // branches hand back the single `out` value; otherwise each branch
                                   // hands back the bindings the if is live in.
                                   std::vector<std::string> results = id.exhaustively_returns
                                                                          ? std::vector<std::string>{"out"}
                                                                          : outs;
                                   std::string fsig = call_signature_name(params.size(), results.size());

                                   lower_fn(then_name, id.then_body);
                                   curr_struct->functions[then_name].in = params;
                                   curr_struct->functions[then_name].out = id.exhaustively_returns
                                                                               ? results
                                                                               : vals2str(id.then_outputs);

                                   lower_fn(else_name, id.else_body);
                                   curr_struct->functions[else_name].in = params;
                                   curr_struct->functions[else_name].out = id.exhaustively_returns
                                                                               ? results
                                                                               : vals2str(id.else_outputs);

                                   curr_struct->functions[frame_name] = create_frame(params, fsig, results.size());

                                   std::string then_ref = fresh_val("ref");
                                   std::string else_ref = fresh_val("ref");
                                   std::string frame_ref = fresh_val("ref");
                                   emit(curr_fn, struct_name, then_name, {}, {then_ref});
                                   emit(curr_fn, struct_name, else_name, {}, {else_ref});
                                   emit(curr_fn, struct_name, frame_name, {}, {frame_ref});

                                   std::string alt1_name = fresh_val("alt");
                                   std::string alt2_name = fresh_val("alt");
                                   emit(curr_fn, fsig, "opt", {cmp1, then_ref}, {alt1_name});
                                   emit(curr_fn, fsig, "opt", {cmp3, else_ref}, {alt2_name});

                                   std::string cont = fresh_val("cont");
                                   emit(curr_fn, fsig, "join", {alt1_name, alt2_name, frame_ref}, {cont});

                                   std::vector call_args{cont};
                                   for (auto &p: params)
                                       call_args.push_back(p);

                                   emit(curr_fn, fsig, "call", call_args, results);
                               },
                               // todo: we should probably stop codegen of curr_fn after hitting return because everything that follows is dead code,
                               [ & ](lin::ret_data &r) {
                                   // what to do with functions that don't "return" anything ?
                                   if (r.arg)
                                       emit(curr_fn, "jsvalue", "move", args2str({r.arg.value()}), {"out"});
                               },
                               [ & ](lin::assert_data &ad) {
                                   emit(curr_fn, "jsvalue", "assert", args2str({ad.arg}), {});
                               },
                               [ & ](lin::call_data &c) {
                                   std::string callee_struct = sema.function_name(c.callee);
                                   std::string f_ref = fresh_val("f_ref");
                                   emit(curr_fn, callee_struct, "run", {}, {f_ref});

                                   std::string fsig = call_signature_name(c.args.size(), 1);
                                   std::vector<std::string> call_args{f_ref};
                                   for (auto &a: args2str(c.args))
                                       call_args.push_back(std::move(a));

                                   emit(curr_fn, fsig, "call", call_args, vals2str({c.target}));
                               },
                               [ & ](lin::loop_data &ld) {
                                   std::vector<std::string> params = vals2str(ld.params);
                                   std::vector<std::string> outs = vals2str(ld.outputs);

                                   // A loop hands back exactly the bindings it carries.
                                   std::string fsig = call_signature_name(params.size(), params.size());

                                   std::string loop_name = fresh_val("loop");
                                   std::string cont_name = fresh_val("loopbody");
                                   std::string exit_name = fresh_val("loopexit");
                                   std::string frame_name = fresh_val("loopframe");

                                   // exit branch: the live params are the loop's result, so they
                                   // travel straight through with nothing to do.
                                   {
                                       function exit_fn{};
                                       exit_fn.in = params;
                                       exit_fn.out = params;
                                       curr_struct->functions[exit_name] = std::move(exit_fn);
                                   }

                                   // continue branch: run one iteration of the body, then tail-recurse
                                   // into loop_name (self-reference by name) with the updated values.
                                   {
                                       std::string self_ref = fresh_val("self");
                                       std::vector rec_args{self_ref};
                                       for (auto &p: vals2str(ld.next_params))
                                           rec_args.push_back(p);

                                       std::vector<std::string> body_outs = fresh_vals("lb", params.size());
                                       std::vector<insn> extra;
                                       extra.push_back(insn{struct_name, loop_name, {}, {self_ref}});
                                       extra.push_back(insn{fsig, "call", rec_args, body_outs});

                                       lower_fn(cont_name, ld.body, extra);
                                       curr_struct->functions[cont_name].in = params;
                                       curr_struct->functions[cont_name].out = body_outs;
                                   }

                                   curr_struct->functions[frame_name] = create_frame(params, fsig, params.size());

                                   // loop_name: the self-recursive dispatcher. Re-runs cond_body and
                                   // the opt/join/call dispatch on *every* call, initial or recursive.
                                   {
                                       std::string cmp1 = fresh_val("cmp"), cmp2 = fresh_val("cmp"), cmp3 = fresh_val(
                                           "cmp");
                                       std::string cont_ref = fresh_val("ref"), exit_ref = fresh_val("ref"), frame_ref =
                                               fresh_val(
                                                   "ref");
                                       std::string alt1 = fresh_val("alt"), alt2 = fresh_val("alt"), joined = fresh_val(
                                           "cont");
                                       std::vector<std::string> loop_outs = fresh_vals("lp", params.size());

                                       std::vector<insn> extra;
                                       extra.push_back(insn{"jsvalue", "dup", args2str({ld.cond}), {cmp1, cmp2}});
                                       extra.push_back(insn{"jsvalue", "not", {cmp2}, {cmp3}});
                                       extra.push_back(insn{struct_name, cont_name, {}, {cont_ref}});
                                       extra.push_back(insn{struct_name, exit_name, {}, {exit_ref}});
                                       extra.push_back(insn{struct_name, frame_name, {}, {frame_ref}});
                                       extra.push_back(insn
                                       {fsig, "opt", {cmp1, cont_ref}, {alt1}});
                                       extra.push_back(insn{fsig, "opt", {cmp3, exit_ref}, {alt2}});
                                       extra.push_back(insn{fsig, "join", {alt1, alt2, frame_ref}, {joined}});

                                       std::vector call_args{joined};
                                       for (auto &p: vals2str(ld.dispatch_args))
                                           call_args.push_back(p);
                                       extra.push_back(insn{fsig, "call", call_args, loop_outs});

                                       lower_fn(loop_name, ld.cond_body, extra);
                                       curr_struct->functions[loop_name].in = params;
                                       curr_struct->functions[loop_name].out = loop_outs;
                                   }

                                   // back in the enclosing function: reference loop_name and call it
                                   // with the current live params -- the bindings the rest of the
                                   // function expects come straight back out of the call.
                                   std::string loop_ref = fresh_val("ref");
                                   emit(curr_fn, struct_name, loop_name, {}, {loop_ref});
                                   std::vector outer_call_args{loop_ref};
                                   for (auto &p: params)
                                       outer_call_args.push_back(p);
                                   emit(curr_fn, fsig, "call", outer_call_args, outs);
                               },
                               [ & ](lin::brk_data &) {
                               },
                               [ & ](lin::cont_data &) {
                               },
                           }, i.data);
            }

            for (auto &e: extra)
                curr_fn.body.push_back(std::move(e));

            curr_struct->functions[name] = std::move(curr_fn);
        }

        structure lower() {
            structure res{.id = struct_name};
            curr_struct = &res;
            lower_fn("run", fn.body);
            curr_struct->functions["run"].in = vals2str(fn.params);

            bool produces_out = false;
            for (auto &insn: curr_struct->functions["run"].body)
                for (auto &o: insn.out)
                    if (o == "out")
                        produces_out = true;

            if (produces_out)
                curr_struct->functions["run"].out = {"out"};

            return *curr_struct;
        }
    };
}
