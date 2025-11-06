#ifndef AST_BUILDER_HPP
#define AST_BUILDER_HPP

#pragma once
#include "ast_def.hpp"

class ASTBuilder {
  public:
    ASTBuilder() = default;
    break_ptr build_break(SourceLocation& loc) const;

    continue_ptr build_continue(SourceLocation& loc) const;

    return_ptr build_return(SourceLocation& loc, expression_ptr_var&& val) const;

    if_ptr build_if(SourceLocation& loc, expression_ptr_var&& cond, scope_err_ptr_var&& scope,
                    std::optional<else_ptr_var>&& else_body) const;

    else_ptr build_else(SourceLocation& loc, std::optional<expression_ptr_var>&& cond,
                        scope_err_ptr_var&& scope) const;

    assign_ptr build_assign(SourceLocation& loc, std::string_view name,
                            expression_ptr_var&& expr) const;

    declare_ptr build_declare(SourceLocation& loc, std::string_view type_name,
                              std::string_view name) const;

    declareassign_ptr build_declareassign(SourceLocation& loc, std::string_view type_name,
                                          std::string_view name, expression_ptr_var&& expr) const;

    integer_ptr build_integer(SourceLocation& loc, int value) const;

    boolean_ptr build_boolean(SourceLocation& loc, bool value) const;

    while_ptr build_while(SourceLocation& loc, expression_ptr_var&& cond,
                          scope_err_ptr_var&& scope) const;

    scope_ptr build_scope(SourceLocation& loc, scope_err_vec_ptr&& stmts) const;

    identifier_ptr build_identifier(SourceLocation& loc, std::string_view& name) const;

    expression_ptr build_expression(SourceLocation& loc, expression_ptr_var&& lhs,
                                    expression_ptr_var&& rhs, Operator op) const;
    struct_ptr build_struct(SourceLocation& loc, std::string_view& name,
                            struct_ptr_var&& body) const;

    program_ptr build_program(SourceLocation& loc, std::vector<statements_ptr_var>&& stmts) const;

    expr_err_ptr build_expr_err(SourceLocation& loc) const;
    stmt_err_ptr build_stmt_err(SourceLocation& loc) const;
};

#endif // AST_BUILDER_HPP
