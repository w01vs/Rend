#ifndef HIR_DEF_HPP
#define HIR_DEF_HPP

#include "ast_def.hpp"
#include "operators.hpp"
#include <string>
#include <variant>

struct VirtualRegisterID {
    int value;
    explicit VirtualRegisterID(int v) : value(v) {}
    int operator++(int other)
    {
        return ++value;
    }
};

using LabelID = int;
using HIRExprFactor = std::variant<VirtualRegisterID, int>;

struct HIRJump {
    LabelID label;
    HIRJump(LabelID label) : label(label) {}
};

struct HIRCondJump {
    HIRExprFactor condition;
    LabelID label;
    HIRCondJump(HIRExprFactor cond, LabelID label) : condition(cond), label(label) {}
};

struct HIRAssign {
    VirtualRegisterID reg;
    HIRExprFactor lhs;
    HIRAssign(VirtualRegisterID vreg, HIRExprFactor expr) : reg(vreg), lhs(expr) {}
};

struct HIRUnaryOp {
    VirtualRegisterID reg;
    HIRExprFactor fac;
    Operator op;

    HIRUnaryOp(VirtualRegisterID vreg, HIRExprFactor expr, Operator op)
        : reg(vreg), fac(expr), op(op)
    {
    }
};

struct HIRBinaryOp {
    VirtualRegisterID reg;
    HIRExprFactor lhs;
    Operator op;
    HIRExprFactor rhs;
    HIRBinaryOp(VirtualRegisterID vreg, HIRExprFactor lhs, Operator op, HIRExprFactor rhs)
        : reg(vreg), lhs(lhs), op(op), rhs(rhs)
    {
    }
};

struct HIRLoad {
    VirtualRegisterID reg;
    std::string_view source;
    HIRLoad(VirtualRegisterID vreg, std::string_view src) : reg(vreg), source(src) {}
};

struct HIRStore {
    VirtualRegisterID reg;
    std::string_view dest;
    HIRStore(VirtualRegisterID vreg, std::string_view dest) : reg(vreg), dest(dest) {}
};

using HIR = std::variant<HIRAssign, HIRUnaryOp, HIRBinaryOp, HIRJump, HIRCondJump, LabelID, HIRLoad,
                         HIRStore>;

std::string_view operator_print(Operator op)
{
    switch(op)
    {
    case Operator::ADD:
        return "+";
    case Operator::SUB:
        return "-";
    case Operator::MUL:
        return "*";
    case Operator::DIV:
        return "/";
    case Operator::MOD:
        return "%";
    case Operator::AND:
        return "&&";
    case Operator::OR:
        return "||";
    case Operator::BAND:
        return "&";
    case Operator::BOR:
        return "|";
    case Operator::XOR:
        return "^";
    case Operator::LSH:
        return "<<";
    case Operator::RSH:
        return ">>";
    case Operator::LESS:
        return "<";
    case Operator::GREATER:
        return ">";
    case Operator::LESSEQ:
        return "<=";
    case Operator::GREATEREQ:
        return ">=";
    case Operator::EQ:
        return "==";
    case Operator::NEQ:
        return "!=";
    case Operator::NOT:
        return "!";
    default:
        return "UNDEFINED";
    }
}

std::string_view hir_expr_print(HIRExprFactor& expr)
{
    return std::visit(Overload{[](VirtualRegisterID& vreg) -> std::string_view {
                                   return std::to_string(vreg.value);
                               },
                               [](int& i) -> std::string_view { return std::to_string(i); },
                               [](auto&&) -> std::string_view { return "UNKNOWN EXPR"; }},
                      expr);
}

std::string_view hir_print(HIR& hir)
{
    return std::visit(
        Overload{
            [](HIRAssign& assign) -> std::string_view {
                return std::to_string(assign.reg.value) + " = " + hir_expr_print(assign.lhs).data();
            },
            [](HIRUnaryOp& unary) -> std::string_view {
                return std::to_string(unary.reg.value) + " = " + operator_print(unary.op).data() +
                       " " + hir_expr_print(unary.fac).data();
            },
            [](HIRBinaryOp& binary) -> std::string_view {
                return std::to_string(binary.reg.value) + " = " +
                       hir_expr_print(binary.lhs).data() + " " + operator_print(binary.op).data() +
                       " " + hir_expr_print(binary.rhs).data();
            },
            [](HIRJump& jump) -> std::string_view { return "goto L" + std::to_string(jump.label); },
            [](HIRCondJump& condjump) -> std::string_view {
                return std::string{"if "} + hir_expr_print(condjump.condition).data() + " goto L" +
                       std::to_string(condjump.label);
            },
            [](LabelID& label) -> std::string_view { return "L" + std::to_string(label) + ":"; },
            [](HIRLoad& load) -> std::string_view {
                return std::to_string(load.reg.value) + " = load " + std::string(load.source);
            },
            [](HIRStore& store) -> std::string_view {
                return "store " + std::to_string(store.reg.value) + " to " +
                       std::string(store.dest);
            },
            [](auto&&) -> std::string_view { return "UNKNOWN HIR"; }},
        hir);
}

#endif // HIR_DEF_HPP
