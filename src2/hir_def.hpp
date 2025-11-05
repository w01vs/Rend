#ifndef HIR_DEF_HPP
#define HIR_DEF_HPP

#include "ast_def.hpp"

using VirtualRegisterID = int;
using LabelID = int;
using HIRExprFactor = std::variant<VirtualRegisterID, int, bool, std::string_view>;

struct HIRJump {
    LabelID label;
};

struct HIRCondJump {
    VirtualRegisterID condition;
    LabelID label;
};


struct HIRAssign {
    VirtualRegisterID reg;
    HIRExprFactor lhs;
};

struct HIRUnaryOp {
    VirtualRegisterID reg;
    HIRExprFactor fac;
    Operator op;
};

struct HIRBinaryOp{
VirtualRegisterID reg;
HIRExprFactor lhs;
Operator op;
HIRExprFactor rhs;
};

#endif // HIR_DEF_HPP
