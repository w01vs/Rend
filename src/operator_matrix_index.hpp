#include "ast_def.hpp"
#include <functional>
#include <utility>

namespace semantics
{
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
} // namespace semantics

namespace std
{
    template <> struct hash<semantics::OperatorMatrixIndex> {
        size_t operator()(const semantics::OperatorMatrixIndex& key) const
        {
            size_t h1 = std::hash<std::shared_ptr<type::BuiltinType>>{}(key.base);
            size_t h2 = std::hash<std::shared_ptr<type::BuiltinType>>{}(key.param);
            return h1 ^ (h2 << 1);
        }
    };
} // namespace std
