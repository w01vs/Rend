#ifndef HIR_DEF_HPP
#define HIR_DEF_HPP

#include "operators.hpp"
#include <string>
#include <variant>

struct VirtualRegisterID {
    int value;
    explicit VirtualRegisterID(int v) : value(v) {}
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

#endif // HIR_DEF_HPP
