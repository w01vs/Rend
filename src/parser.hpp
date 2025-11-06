#ifndef PARSER_HPP
#define PARSER_HPP

#include "ast_builder.hpp"
#include "ast_def.hpp"
#include "errors.hpp"
#include "tokens.hpp"
#include "tokenstream.hpp"
#include <iostream>
#include <memory>
#include <unordered_map>

enum class BuiltinType : char {
    INT,
    BOOL,
};

class Parser {
  public:
    Parser(TokenStream& stream, ErrorReporter& reporter);

    program_ptr&& parse();

  private:
    static const std::unordered_map<TokenType, int> PRECEDENCE;
    static const std::unordered_map<TokenType, Operator> TOKEN_OP;

    type::TypeRegistry& type_registry_;
    TokenStream& stream_;
    ErrorReporter& reporter_;
    ASTBuilder builder_;

    statements_ptr_var parse_statement() const;

    expression_ptr_var parse_expression() const;

    statements_ptr_var parse_from_ident() const;

    statements_ptr_var parse_assign() const;

    statements_ptr_var parse_declassign() const;

    statements_ptr_var parse_builtin_var(const BuiltinType type) const;

    statements_ptr_var parse_return() const;

    statements_ptr_var parse_while() const;

    statements_ptr_var parse_if() const;

    std::optional<else_ptr_var> parse_else() const;

    statements_ptr_var parse_struct() const;

    struct_body_var parse_struct_declassign() const;

    std::optional<struct_body_var> struct_helper() const;

    scope_err_ptr_var parse_scope() const;

    void synchronize_tokens() const;

    bool string_to_bool(const std::string_view& str) const;

    Operator token_to_operator(TokenType type) const;
};
#endif // PARSER_HPP
