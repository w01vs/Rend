#ifndef AST_DEF_HPP
#define AST_DEF_HPP

#pragma once
#include "operators.hpp"
#include "shared/sourcelocation.hpp"
#include "type.hpp"
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

struct ASTNode {
    SourceLocation loc;
    ASTNode(SourceLocation& loc) : loc(loc) {}
};

struct ASTStatementBase : public ASTNode {
    ASTStatementBase(SourceLocation& loc) : ASTNode(loc) {}
};

struct ASTExpressionBase : public ASTNode {
    int label;
    std::shared_ptr<type::BuiltinType> type;
    ASTExpressionBase(SourceLocation& loc) : ASTNode(loc), label(-1), type(nullptr) {}
};

using expression_base_ptr = std::unique_ptr<ASTExpressionBase>;

struct ASTStatementError : public ASTStatementBase {
    ASTStatementError(SourceLocation& loc) : ASTStatementBase(loc) {}
};

using stmt_err_ptr = std::unique_ptr<ASTStatementError>;

struct ASTExpressionError : public ASTExpressionBase {
    ASTExpressionError(SourceLocation& loc) : ASTExpressionBase(loc) {}
};

using expr_err_ptr = std::unique_ptr<ASTExpressionError>;

struct ASTIdentifier : public ASTExpressionBase {
    std::string_view name;
    int scope_id;
    ASTIdentifier(SourceLocation& loc, std::string_view name)
        : ASTExpressionBase(loc), name(std::move(name)), scope_id(-1)
    {
    }
};

using identifier_ptr = std::unique_ptr<ASTIdentifier>;

struct ASTInteger : public ASTExpressionBase {
    int value;
    ASTInteger(SourceLocation& loc, int value) : ASTExpressionBase(loc), value(value) {}
};

using integer_ptr = std::unique_ptr<ASTInteger>;

struct ASTBoolean : public ASTExpressionBase {
    bool value;
    ASTBoolean(SourceLocation& loc, bool value) : ASTExpressionBase(loc), value(value) {}
};

using boolean_ptr = std::unique_ptr<ASTBoolean>;

struct ASTExpression;

using expression_ptr = std::unique_ptr<ASTExpression>;

using expression_ptr_var =
    std::variant<expression_ptr, identifier_ptr, integer_ptr, boolean_ptr, expr_err_ptr>;

struct ASTExpression : public ASTExpressionBase {
    expression_ptr_var lhs;
    expression_ptr_var rhs;
    Operator op;
    int weight; // for Sethi-Ullman algo
    ASTExpression(SourceLocation& loc, expression_ptr_var&& lhs, expression_ptr_var&& rhs,
                  Operator& op)
        : ASTExpressionBase(loc), lhs(std::move(lhs)), rhs(std::move(rhs)), op(op)
    {
    }
};

struct ASTReturn : public ASTStatementBase {
    expression_ptr_var val;
    ASTReturn(SourceLocation& loc, expression_ptr_var&& val)
        : ASTStatementBase(loc), val(std::move(val))
    {
    }
};

using return_ptr = std::unique_ptr<ASTReturn>;

struct ASTAssign : public ASTStatementBase {
    identifier_ptr ident;
    expression_ptr_var expr;
    ASTAssign(SourceLocation& loc, identifier_ptr&& ident, expression_ptr_var&& expr)
        : ASTStatementBase(loc), ident(std::move(ident)), expr(std::move(expr))
    {
    }
};

using assign_ptr = std::unique_ptr<ASTAssign>;

struct ASTDeclareAssign : public ASTStatementBase {
    std::shared_ptr<type::BuiltinType> type;
    std::string_view type_name;
    identifier_ptr ident;
    expression_ptr_var expr;
    ASTDeclareAssign(SourceLocation& loc, std::string_view type, identifier_ptr&& ident,
                     expression_ptr_var&& expr)
        : ASTStatementBase(loc), type_name(std::move(type)), ident(std::move(ident)),
          expr(std::move(expr)), type(nullptr)
    {
    }
};

using declareassign_ptr = std::unique_ptr<ASTDeclareAssign>;

struct ASTDeclaration : public ASTStatementBase {
    std::shared_ptr<type::BuiltinType> type;
    std::string_view type_name;
    identifier_ptr ident;
    ASTDeclaration(SourceLocation& loc, std::string_view type, identifier_ptr&& ident)
        : ASTStatementBase(loc), type_name(std::move(type)), ident(std::move(ident)), type(nullptr)
    {
    }
};

using declare_ptr = std::unique_ptr<ASTDeclaration>;

struct ASTScope;

using scope_ptr = std::unique_ptr<ASTScope>;

using scope_err_ptr_var = std::variant<scope_ptr, stmt_err_ptr>;

struct ASTWhile : public ASTStatementBase {
    expression_ptr_var condition;
    scope_err_ptr_var scope;
    ASTWhile(SourceLocation& loc, expression_ptr_var&& condition, scope_err_ptr_var&& scope)
        : ASTStatementBase(loc), condition(std::move(condition)), scope(std::move(scope))
    {
    }

    ASTWhile(std::unique_ptr<ASTWhile>&& _while)
        : condition(std::move(_while->condition)), scope(std::move(_while->scope)),
          ASTStatementBase(_while->loc)
    {
    }
};

using while_ptr = std::unique_ptr<ASTWhile>;

struct ASTContinue : public ASTStatementBase {
    ASTContinue(SourceLocation& loc) : ASTStatementBase(loc) {}
};

using continue_ptr = std::unique_ptr<ASTContinue>;

struct ASTBreak : public ASTStatementBase {
    ASTBreak(SourceLocation& loc) : ASTStatementBase(loc) {}
};

using break_ptr = std::unique_ptr<ASTBreak>;

struct ASTIf;
struct ASTElse;

using if_ptr = std::unique_ptr<ASTIf>;
using else_ptr_var = std::variant<std::unique_ptr<ASTElse>, stmt_err_ptr>;

struct ASTElse : public ASTStatementBase {
    std::optional<expression_ptr_var> condition;
    scope_err_ptr_var scope;
    std::optional<else_ptr_var> else_if_clause;
    ASTElse(SourceLocation& loc, std::optional<expression_ptr_var>&& condition,
            scope_err_ptr_var&& scope, std::optional<else_ptr_var>&& else_if_clause)
        : ASTStatementBase(loc), condition(std::move(condition)), scope(std::move(scope)),
          else_if_clause(std::move(else_if_clause))
    {
    }

    ASTElse(std::unique_ptr<ASTElse>&& _else)
        : condition(std::move(_else->condition)), scope(std::move(_else->scope)),
          ASTStatementBase(_else->loc), else_if_clause(std::move(_else->else_if_clause))
    {
    }
};

using else_ptr = std::unique_ptr<ASTElse>;


struct ASTIf : public ASTStatementBase {
    expression_ptr_var condition;
    scope_err_ptr_var scope;
    std::optional<else_ptr_var> else_clause;
    ASTIf(SourceLocation& loc, expression_ptr_var&& condition, scope_err_ptr_var&& scope,
          std::optional<else_ptr_var>&& else_clause)
        : ASTStatementBase(loc), condition(std::move(condition)), scope(std::move(scope)),
          else_clause(std::move(else_clause))
    {
    }

    ASTIf(std::unique_ptr<ASTIf>&& _if)
        : condition(std::move(_if->condition)), scope(std::move(_if->scope)),
          else_clause(std::move(_if->else_clause)), ASTStatementBase(_if->loc)
    {
    }
};

struct ASTStruct;
using struct_ptr = std::unique_ptr<ASTStruct>;

using statements_ptr_var =
    std::variant<scope_ptr, break_ptr, continue_ptr, return_ptr, if_ptr, while_ptr,
                 struct_ptr, declareassign_ptr, declare_ptr, assign_ptr, stmt_err_ptr>;

// Struct body will change in future
using struct_body_var = std::variant<declare_ptr, declareassign_ptr, stmt_err_ptr>;

using struct_ptr_var = std::variant<std::vector<struct_body_var>, stmt_err_ptr>;

struct ASTStruct : public ASTStatementBase {
    std::string_view name;
    struct_ptr_var members;
    ASTStruct(SourceLocation& loc, struct_ptr_var&& members, std::string_view& name)
        : ASTStatementBase(loc), members(std::move(members)), name(name)
    {
    }
};

using stmt_vec_err_ptr = std::variant<std::vector<statements_ptr_var>, stmt_err_ptr>;

struct ASTScope : public ASTStatementBase {
    stmt_vec_err_ptr stmts; // all statements inside scope
    ASTScope(SourceLocation& loc, stmt_vec_err_ptr&& stmts)
        : ASTStatementBase(loc), stmts(std::move(stmts))
    {
    }
    ASTScope(std::unique_ptr<ASTScope>&& scope)
        : ASTStatementBase(scope->loc), stmts(std::move(scope->stmts))
    {
    }
};

struct ASTProgram : public ASTNode {
    std::vector<statements_ptr_var> stmts;
    ASTProgram(SourceLocation& loc, std::vector<statements_ptr_var>&& stmts)
        : ASTNode(loc), stmts(std::move(stmts))
    {
    }
};

using program_ptr = std::unique_ptr<ASTProgram>;


#endif // AST_DEF_HPP
