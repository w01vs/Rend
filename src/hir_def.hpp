#ifndef HIR_DEF_HPP
#define HIR_DEF_HPP

#include "operators.hpp"
#include <string>
#include <variant>

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

struct HIRBinaryOp {
    VirtualRegisterID reg;
    HIRExprFactor lhs;
    Operator op;
    HIRExprFactor rhs;
};

struct HIRLoad {
    VirtualRegisterID reg;
    std::string_view source;
};

struct HIRStore {
    std::string_view dest;
    VirtualRegisterID reg;
};

#endif // HIR_DEF_HPP
