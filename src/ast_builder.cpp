#include "ast_builder.hpp"

break_ptr ASTBuilder::build_break(SourceLocation& loc) const
{
    return std::make_unique<ASTBreak>(loc);
}

continue_ptr ASTBuilder::build_continue(SourceLocation& loc) const
{
    return std::make_unique<ASTContinue>(loc);
}

return_ptr ASTBuilder::build_return(SourceLocation& loc, expression_ptr_var&& val) const
{
    return std::make_unique<ASTReturn>(loc, std::move(val));
}

if_ptr ASTBuilder::build_if(SourceLocation& loc, expression_ptr_var&& cond,
                            scope_err_ptr_var&& scope,
                            std::optional<else_ptr_var>&& else_body) const
{
    return std::make_unique<ASTIf>(loc, std::move(cond), std::move(scope), std::move(else_body));
}

else_ptr ASTBuilder::build_else(SourceLocation& loc, std::optional<expression_ptr_var>&& cond,
                                scope_err_ptr_var&& scope) const
{
    return std::make_unique<ASTElse>(loc, std::move(cond), std::move(scope));
}

assign_ptr ASTBuilder::build_assign(SourceLocation& loc, std::string_view name,
                                    expression_ptr_var&& expr) const
{
    return std::make_unique<ASTAssign>(loc, std::move(name), std::move(expr));
}

declare_ptr ASTBuilder::build_declare(SourceLocation& loc, std::string_view type_name,
                                      std::string_view name) const
{
    return std::make_unique<ASTDeclaration>(loc, std::move(type_name), std::move(name));
}

declareassign_ptr ASTBuilder::build_declareassign(SourceLocation& loc, std::string_view type_name,
                                                  std::string_view name,
                                                  expression_ptr_var&& expr) const
{
    return std::make_unique<ASTDeclareAssign>(loc,
                                              std::move(type_name),
                                              std::move(name),
                                              std::move(expr));
}

integer_ptr ASTBuilder::build_integer(SourceLocation& loc, int value) const
{
    return std::make_unique<ASTInteger>(loc, value);
}

boolean_ptr ASTBuilder::build_boolean(SourceLocation& loc, bool value) const
{
    return std::make_unique<ASTBoolean>(loc, value);
}

while_ptr ASTBuilder::build_while(SourceLocation& loc, expression_ptr_var&& cond,
                                  scope_err_ptr_var&& scope) const
{
    return std::make_unique<ASTWhile>(loc, std::move(cond), std::move(scope));
}

scope_ptr ASTBuilder::build_scope(SourceLocation& loc,
                                  scope_err_vec_ptr&& stmts) const
{
    return std::make_unique<ASTScope>(loc, std::move(stmts));
}

struct_ptr ASTBuilder::build_struct(SourceLocation& loc, std::string_view& name,
                                    struct_ptr_var&& body) const
{
    return std::make_unique<ASTStruct>(loc, std::move(body), name);
}

program_ptr ASTBuilder::build_program(SourceLocation& loc,
                                      std::vector<statements_ptr_var>&& stmts) const
{
    return std::make_unique<ASTProgram>(loc, std::move(stmts));
}

identifier_ptr ASTBuilder::build_identifier(SourceLocation& loc, std::string_view& name) const
{
    return std::make_unique<ASTIdentifier>(loc, name);
}

expression_ptr ASTBuilder::build_expression(SourceLocation& loc,
                                            expression_ptr_var&& lhs,
                                            expression_ptr_var&& rhs,
                                            Operator op) const
{
    return std::make_unique<ASTExpression>(loc, std::move(lhs), std::move(rhs), op);
}

expr_err_ptr ASTBuilder::build_expr_err(SourceLocation& loc) const
{
    return std::make_unique<ASTExpressionError>(loc);
}

stmt_err_ptr ASTBuilder::build_stmt_err(SourceLocation& loc) const
{
    return std::make_unique<ASTStatementError>(loc);
}
