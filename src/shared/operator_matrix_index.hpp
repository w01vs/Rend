#pragma once
#include <functional>
#include <utility>
#include "type.hpp"
#include "operators.hpp"

struct Var {
    std::string_view name;
    std::shared_ptr<type::BuiltinType> type;
};

struct OperatorMatrixIndex {
    std::shared_ptr<type::BuiltinType> base;
    Operator op;
    std::shared_ptr<type::BuiltinType> param;

    bool operator==(const OperatorMatrixIndex& other) const
    {
        return base == other.base && op == other.op && param == other.param;
    }
};

struct OperatorResult {
    std::shared_ptr<type::BuiltinType> result;
    // coerce rules here, e.g. type promotion (int -> float) or division type (integer div)
};

namespace std
{
    template <> struct hash<OperatorMatrixIndex> {
        size_t operator()(const OperatorMatrixIndex& key) const
        {
            size_t h1 = std::hash<std::shared_ptr<type::BuiltinType>>{}(key.base);
            size_t h2 = std::hash<std::shared_ptr<type::BuiltinType>>{}(key.param);
            return h1 ^ (h2 << 1);
        }
    };
} // namespace std
