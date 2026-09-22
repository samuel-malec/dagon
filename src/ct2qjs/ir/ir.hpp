#pragma once

#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <vector>

#include "../frontend/symtab.hpp"
#include "../../cthu_core/types.hpp"

namespace qthu::ct2qjs {
    struct insn_key {
        atom stru;
        atom op;

        bool operator<(const insn_key &rhs) const {
            if (stru.index != rhs.stru.index)
                return stru.index < rhs.stru.index;
            return op.index < rhs.op.index;
        }
    };

    struct resolved_insn {
        enum class kind_t {
            builtin,
            fn_ref,
            fn_call,
            fn_opt,
            fn_join,
        };

        kind_t kind = kind_t::builtin;
        atom structure{};
        atom operation{};

        atom target{};
        uint32_t target_fn_id = std::numeric_limits<uint32_t>::max();
        std::vector<atom> in;
        std::vector<atom> out;
    };

    struct lowered_insn {
        resolved_insn resolved;
        std::vector<uint32_t> slots_in;
        std::vector<uint32_t> slots_out;
    };

    struct trampoline_t {
        uint32_t continue_id = 0;
        uint32_t exit_ref_idx = 0;
        uint32_t continue_ref_idx = 0;
        uint32_t frame_ref_idx = 0;
        uint32_t opt_continue_idx = 0;
        uint32_t opt_exit_idx = 0;
        uint32_t join_idx = 0;
        uint32_t call_idx = 0;
        uint32_t continue_tail_idx = 0;
        uint32_t continue_self_ref_idx = 0;
    };

    struct fn_meta {
        uint32_t id = 0;
        insn_key key{};
        std::vector<atom> in;
        std::vector<atom> out;

        std::vector<resolved_insn> body;
        std::vector<lowered_insn> lowered;

        std::vector<uint32_t> in_param_slots;
        std::vector<uint32_t> out_param_slots;
        uint32_t slot_size = 0;
    };

    struct program {
        symtab &st;
        std::vector<fn_meta> fns{};
        std::map<insn_key, uint32_t> key_fn{};
        std::map<insn_key, atom> builtins{};
        std::map<uint32_t, trampoline_t> trampolines{};

        void collect_fns() {
            for (const auto &[struct_atom, structure]: st.structures) {
                for (const auto &[func_atom, func]: structure.functions) {
                    auto metadata = fn_meta{
                        static_cast<uint32_t>(fns.size()),
                        {struct_atom, func_atom},
                        func.in,
                        func.out,
                    };

                    key_fn[metadata.key] = metadata.id;
                    fns.push_back(std::move(metadata));
                }
            }
        }

        void collect_builtins() {
            for (const auto &[satom, structure]: st.structures)
                for (const auto &[op, builtin_name]: structure.builtin_ops)
                    builtins[{satom, op}] = builtin_name;
        }

        void alloc_slots() {
            for (auto &meta: fns) {
                std::map<atom, std::vector<uint32_t> > versions;
                uint32_t next_slot = 0;
                std::vector<uint32_t> free_slots;

                auto alloc_slot = [&]() -> uint32_t {
                    if (!free_slots.empty()) {
                        const uint32_t s = free_slots.back();
                        free_slots.pop_back();
                        return s;
                    }
                    return next_slot++;
                };

                auto pop_version = [&](atom name) -> uint32_t {
                    auto it = versions.find(name);
                    if (it == versions.end() || it->second.empty())
                        throw std::runtime_error(
                            std::string("slot alloc underflow for name: ") +
                            std::string(st.name_of(name)));

                    const uint32_t s = it->second.back();
                    it->second.pop_back();
                    return s;
                };

                auto push_version = [&](atom name, uint32_t slot) {
                    versions[name].push_back(slot);
                };

                for (const auto &p: meta.in) {
                    const uint32_t s = alloc_slot();
                    push_version(p, s);
                    meta.in_param_slots.push_back(s);
                }

                for (const auto &insn: meta.body) {
                    lowered_insn low;
                    low.resolved = insn;

                    std::vector<uint32_t> to_free;
                    to_free.reserve(insn.in.size());

                    for (const auto &iatom: insn.in) {
                        const uint32_t s = pop_version(iatom);
                        low.slots_in.push_back(s);
                        to_free.push_back(s);
                    }

                    for (const uint32_t s: to_free)
                        free_slots.push_back(s);

                    for (const auto &oatom: insn.out) {
                        const uint32_t s = alloc_slot();
                        push_version(oatom, s);
                        low.slots_out.push_back(s);
                    }

                    meta.lowered.push_back(std::move(low));
                }

                for (const auto &o: meta.out) {
                    auto it = versions.find(o);
                    if (it == versions.end() || it->second.empty()) {
                        throw std::runtime_error(
                            std::string("function output name not produced: ") +
                            std::string(st.name_of(meta.key.stru)) + "::" +
                            std::string(st.name_of(meta.key.op)) + " -> " +
                            std::string(st.name_of(o)));
                    }
                    meta.out_param_slots.push_back(it->second.back());
                }

                meta.slot_size = next_slot;
            }
        }

        size_t sig_side_arity(const std::vector<atom> &side) const {
            if (side.size() == 1 && st.name_of(side[0]) == "∅")
                return 0;
            return side.size();
        }

        std::optional<std::pair<size_t, size_t> > find_op_arity_in_signature(
            atom sig_name, atom op, std::set<atom> &seen) {
            if (!seen.insert(sig_name).second)
                return std::nullopt;

            auto it = st.signatures.find(sig_name);
            if (it == st.signatures.end())
                return std::nullopt;

            if (auto dit = it->second.defs.find(op); dit != it->second.defs.end())
                return std::pair{sig_side_arity(dit->second.in), sig_side_arity(dit->second.out)};

            for (auto &[parent, parent_args]: it->second.inherits)
                if (auto found = find_op_arity_in_signature(parent, op, seen))
                    return found;

            return std::nullopt;
        }

        std::optional<std::pair<size_t, size_t> > declared_arity(atom stru, atom op) {
            auto sit = st.structures.find(stru);
            if (sit == st.structures.end())
                return std::nullopt;

            for (auto &sig_inst: sit->second.signatures) {
                std::set<atom> seen;
                if (auto found = find_op_arity_in_signature(sig_inst.signature, op, seen))
                    return found;
            }

            return std::nullopt;
        }

        void validate_arity(const insn_t &insn, size_t expected_in, size_t expected_out) {
            if (insn.in.size() == expected_in && insn.out.size() == expected_out)
                return;

            throw std::runtime_error(
                std::string(st.name_of(insn.structure)) + "::" + std::string(st.name_of(insn.operation)) +
                ": expects " + std::to_string(expected_in) + " input(s) and " +
                std::to_string(expected_out) + " output(s) per its declared signature, got " +
                std::to_string(insn.in.size()) + " and " + std::to_string(insn.out.size()));
        }

        resolved_insn classify(const insn_t &insn) {
            auto op_name = st.name_of(insn.operation);
            if (op_name == "call") {
                if (insn.in.empty() || insn.out.size() != 1)
                    throw std::runtime_error(
                        std::string(st.name_of(insn.structure)) + "::call: expects at least 1 input "
                        "(the callee) and exactly 1 output, got " + std::to_string(insn.in.size()) +
                        " and " + std::to_string(insn.out.size()));

                return resolved_insn{
                    .kind = resolved_insn::kind_t::fn_call,
                    .structure = insn.structure,
                    .operation = insn.operation,
                    .target = insn.operation,
                    .in = insn.in,
                    .out = insn.out,
                };
            }

            if (op_name == "opt") {
                // emit_fn_opt reads exactly slots_in[0] (cond) and slots_in[1]
                // (the function value), writes exactly slots_out[0].
                if (insn.in.size() != 2 || insn.out.size() != 1)
                    throw std::runtime_error(
                        std::string(st.name_of(insn.structure)) + "::opt: expects exactly 2 inputs "
                        "(cond, fn ref) and 1 output, got " + std::to_string(insn.in.size()) +
                        " and " + std::to_string(insn.out.size()));

                return resolved_insn{
                    .kind = resolved_insn::kind_t::fn_opt,
                    .structure = insn.structure,
                    .operation = insn.operation,
                    .target = insn.operation,
                    .in = insn.in,
                    .out = insn.out,
                };
            }

            if (op_name == "join" && st.name_of(insn.structure).starts_with("f")) {
                if (insn.in.size() < 2 || insn.out.size() != 1)
                    throw std::runtime_error(
                        std::string(st.name_of(insn.structure)) + "::join: expects at least 2 inputs "
                        "(the two dispatch alternatives) and 1 output, got " + std::to_string(insn.in.size()) +
                        " and " + std::to_string(insn.out.size()));

                return resolved_insn{
                    .kind = resolved_insn::kind_t::fn_join,
                    .structure = insn.structure,
                    .operation = insn.operation,
                    .target = insn.operation,
                    .in = insn.in,
                    .out = insn.out,
                };
            }

            insn_key key{insn.structure, insn.operation};

            // add cons_ on demand
            if (op_name.starts_with("cons_") && st.name_of(insn.structure) == "jsvalue") {
                std::string builtin_name = "qjs_val_";
                builtin_name += st.name_of(key.op);
                auto atom = st.get(builtin_name);
                builtins[key] = atom;
            }

            if (auto it = builtins.find(key); it != builtins.end()) {
                // Every cons_<suffix> variant (cons_5, cons_true, cons_str_2, ...)
                // shares the base `cons` op's declared arity -- there's no separate
                // signature entry for each suffix, since the suffix isn't an
                // operand, it's part of the opcode name.
                atom sig_op = op_name.starts_with("cons_") ? st.get("cons") : insn.operation;
                if (auto expected = declared_arity(insn.structure, sig_op))
                    validate_arity(insn, expected->first, expected->second);

                return resolved_insn{
                    resolved_insn::kind_t::builtin,
                    insn.structure,
                    insn.operation,
                    it->second,
                    std::numeric_limits<uint32_t>::max(),
                    insn.in,
                    insn.out,
                };
            }

            if (auto it = key_fn.find(key); it != key_fn.end()) {
                // Referencing a declared function by name (`struct_name fn_name ->
                // ref`) is a distinct thing from calling it: it always produces one
                // first-class function value and never takes operands, regardless
                // of the referenced function's own parameter count -- confirmed
                // against emit_insn's fn_ref case, which reads no inputs at all.
                validate_arity(insn, 0, 1);

                return resolved_insn{
                    resolved_insn::kind_t::fn_ref,
                    insn.structure,
                    insn.operation,
                    insn.operation,
                    it->second,
                    insn.in,
                    insn.out,
                };
            }

            throw std::runtime_error(
                std::string("unresolved instruction: ") +
                std::string(st.name_of(insn.structure)) + "::" +
                std::string(st.name_of(insn.operation))
            );
        }

        void resolve_instructions() {
            for (auto &meta: fns) {
                auto &structure = st.structures.at(meta.key.stru);
                auto &function = structure.functions.at(meta.key.op);

                for (const auto &insn: function.body)
                    meta.body.push_back(classify(insn));
            }
        }

        // Index of the instruction in `body` whose sole/first output atom is `a`
        // (every kind we search for here -- fn_ref/fn_opt/fn_join -- has exactly
        // one output). Atoms are single-assignment within one function body (every
        // name `structure_builder` emits is fresh), so this is unambiguous.
        static std::optional<size_t> find_producer(const std::vector<resolved_insn> &body, atom a) {
            for (size_t i = 0; i < body.size(); ++i)
                if (!body[i].out.empty() && body[i].out[0] == a)
                    return i;
            return std::nullopt;
        }

        struct continue_match {
            size_t tail_idx; // g's final fn_call -- eliminated (becomes the loop-back)
            size_t self_ref_idx; // the fn_ref feeding it -- also eliminated (never called)
        };

        // Does `g` unconditionally tail-call back into `dispatcher_id` -- i.e. is
        // `g` a valid "continue" branch for that dispatcher?
        static std::optional<continue_match> continue_shape(const fn_meta &g, uint32_t dispatcher_id,
                                                            size_t dispatcher_argc) {
            if (g.out.size() != 1 || g.body.empty() || g.in.size() != dispatcher_argc)
                return std::nullopt;

            const resolved_insn &tail = g.body.back();
            if (tail.kind != resolved_insn::kind_t::fn_call)
                return std::nullopt;
            if (tail.out.empty() || tail.out[0] != g.out[0])
                return std::nullopt;
            if (tail.in.empty() || tail.in.size() - 1 != dispatcher_argc)
                return std::nullopt;

            auto ref_pos = find_producer(g.body, tail.in[0]);
            if (!ref_pos || g.body[*ref_pos].kind != resolved_insn::kind_t::fn_ref)
                return std::nullopt;
            if (g.body[*ref_pos].target_fn_id != dispatcher_id)
                return std::nullopt;

            return continue_match{g.body.size() - 1, *ref_pos};
        }

        void find_trampolines() {
            for (auto &meta: fns) {
                if (meta.out.size() != 1 || meta.body.empty())
                    continue;

                const resolved_insn &call_insn = meta.body.back();
                if (call_insn.kind != resolved_insn::kind_t::fn_call)
                    continue;
                if (call_insn.out.empty() || call_insn.out[0] != meta.out[0])
                    continue;
                if (call_insn.in.empty() || call_insn.in.size() - 1 != meta.in.size())
                    continue;

                auto join_pos = find_producer(meta.body, call_insn.in[0]);
                if (!join_pos || meta.body[*join_pos].kind != resolved_insn::kind_t::fn_join)
                    continue;
                const resolved_insn &join_insn = meta.body[*join_pos];
                if (join_insn.in.size() < 3)
                    continue;

                auto opt1_pos = find_producer(meta.body, join_insn.in[0]);
                auto opt2_pos = find_producer(meta.body, join_insn.in[1]);
                if (!opt1_pos || !opt2_pos)
                    continue;
                if (meta.body[*opt1_pos].kind != resolved_insn::kind_t::fn_opt)
                    continue;
                if (meta.body[*opt2_pos].kind != resolved_insn::kind_t::fn_opt)
                    continue;
                const resolved_insn &opt1 = meta.body[*opt1_pos];
                const resolved_insn &opt2 = meta.body[*opt2_pos];
                if (opt1.in.size() < 2 || opt2.in.size() < 2)
                    continue;

                auto ref1_pos = find_producer(meta.body, opt1.in[1]);
                auto ref2_pos = find_producer(meta.body, opt2.in[1]);
                if (!ref1_pos || !ref2_pos)
                    continue;
                if (meta.body[*ref1_pos].kind != resolved_insn::kind_t::fn_ref)
                    continue;
                if (meta.body[*ref2_pos].kind != resolved_insn::kind_t::fn_ref)
                    continue;

                auto frame_pos = find_producer(meta.body, join_insn.in[2]);
                if (!frame_pos || meta.body[*frame_pos].kind != resolved_insn::kind_t::fn_ref)
                    continue;

                const uint32_t target1 = meta.body[*ref1_pos].target_fn_id;
                const uint32_t target2 = meta.body[*ref2_pos].target_fn_id;
                if (target1 >= fns.size() || target2 >= fns.size())
                    continue;

                auto tail1 = continue_shape(fns[target1], meta.id, meta.in.size());
                auto tail2 = continue_shape(fns[target2], meta.id, meta.in.size());
                if (tail1.has_value() == tail2.has_value())
                    continue; // need exactly one match -- ambiguous or neither

                const uint32_t continue_id = tail1 ? target1 : target2;
                const size_t continue_tail_idx = tail1 ? tail1->tail_idx : tail2->tail_idx;
                const size_t continue_self_ref_idx = tail1 ? tail1->self_ref_idx : tail2->self_ref_idx;
                const size_t continue_ref_idx = tail1 ? *ref1_pos : *ref2_pos;
                const size_t opt_continue_idx = tail1 ? *opt1_pos : *opt2_pos;
                const size_t exit_ref_idx = tail1 ? *ref2_pos : *ref1_pos;
                const size_t opt_exit_idx = tail1 ? *opt2_pos : *opt1_pos;

                size_t referrers = 0;
                for (auto &other: fns)
                    for (auto &insn: other.body)
                        if (insn.kind == resolved_insn::kind_t::fn_ref && insn.target_fn_id == continue_id)
                            ++referrers;
                if (referrers != 1)
                    continue;

                trampolines[meta.id] = trampoline_t{
                    .continue_id = continue_id,
                    .exit_ref_idx = static_cast<uint32_t>(exit_ref_idx),
                    .continue_ref_idx = static_cast<uint32_t>(continue_ref_idx),
                    .frame_ref_idx = static_cast<uint32_t>(*frame_pos),
                    .opt_continue_idx = static_cast<uint32_t>(opt_continue_idx),
                    .opt_exit_idx = static_cast<uint32_t>(opt_exit_idx),
                    .join_idx = static_cast<uint32_t>(*join_pos),
                    .call_idx = static_cast<uint32_t>(meta.body.size() - 1),
                    .continue_tail_idx = static_cast<uint32_t>(continue_tail_idx),
                    .continue_self_ref_idx = static_cast<uint32_t>(continue_self_ref_idx),
                };
            }
        }

        void lower_to_ir() {
            collect_fns();
            collect_builtins();
            resolve_instructions();
            find_trampolines();
            alloc_slots();
        }

        void print() const {
            for (auto &fn: fns) {
                std::cout << "fn " << st.name_of(fn.key.stru) << "::" << st.name_of(fn.key.op) << '\n';
                for (auto &l: fn.lowered) {
                    std::cout << "      ";
                    switch (l.resolved.kind) {
                        case resolved_insn::kind_t::builtin:
                            std::cout << "(builtin)";
                            break;
                        case resolved_insn::kind_t::fn_call:
                            std::cout << "(call)";
                            break;
                        case resolved_insn::kind_t::fn_join:
                            std::cout << "(join)";
                            break;
                        case resolved_insn::kind_t::fn_opt:
                            std::cout << "(opt)";
                            break;
                        case resolved_insn::kind_t::fn_ref:
                            std::cout << "(fn_ref)";
                            break;
                        default:
                            std::cout << "(unknown)";
                            break;
                    }

                    std::cout << " " << st.name_of(l.resolved.target) << " in: [ ";
                    for (auto &in: l.slots_in)
                        std::cout << in << " ";
                    std::cout << "] out: [ ";
                    for (auto &out: l.slots_out)
                        std::cout << out << " ";
                    std::cout << "]\n";
                }
            }
        }
    };
}
