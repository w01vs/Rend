#include "parser.hpp"

const std::unordered_map<TokenType, int> Parser::PRECEDENCE{
    {TokenType::OP_LOGICAL_OR, 1},
    {TokenType::OP_LOGICAL_AND, 2},
    {TokenType::OP_BITWISE_OR, 3},
    {TokenType::OP_BITWISE_XOR, 4},
    {TokenType::OP_BITWISE_AND, 5},
    {TokenType::OP_EQUAL, 6},
    {TokenType::OP_NOT_EQUAL, 6},
    {TokenType::OP_GREATER, 7},
    {TokenType::OP_LESS, 7},
    {TokenType::OP_LESS_EQUAL, 7},
    {TokenType::OP_GREATER_EQUAL, 7},
    {TokenType::OP_LSH, 8},
    {TokenType::OP_RSH, 8},
    {TokenType::OP_ADD, 9},
    {TokenType::OP_SUB, 9},
    {TokenType::OP_MUL, 10},
    {TokenType::OP_DIV, 10},
    {TokenType::OP_MOD, 10},
};

const std::unordered_map<TokenType, Operator> Parser::TOKEN_OP{
    {TokenType::OP_LOGICAL_OR, Operator::OR},
    {TokenType::OP_LOGICAL_AND, Operator::AND},
    {TokenType::OP_BITWISE_OR, Operator::BOR},
    {TokenType::OP_BITWISE_XOR, Operator::XOR},
    {TokenType::OP_BITWISE_AND, Operator::BAND},
    {TokenType::OP_EQUAL, Operator::EQ},
    {TokenType::OP_NOT_EQUAL, Operator::NEQ},
    {TokenType::OP_GREATER, Operator::GREATER},
    {TokenType::OP_LESS, Operator::LESS},
    {TokenType::OP_LESS_EQUAL, Operator::LESSEQ},
    {TokenType::OP_GREATER_EQUAL, Operator::GREATEREQ},
    {TokenType::OP_LSH, Operator::LSH},
    {TokenType::OP_RSH, Operator::RSH},
    {TokenType::OP_ADD, Operator::ADD},
    {TokenType::OP_SUB, Operator::SUB},
    {TokenType::OP_MUL, Operator::MUL},
    {TokenType::OP_DIV, Operator::DIV},
    {TokenType::OP_MOD, Operator::MOD},
};

Parser::Parser(TokenStream& stream, ErrorReporter& reporter)
    : stream_(stream), reporter_(reporter), builder_(),
      type_registry_(type::TypeRegistry::instance())
{
}

// Will return the appropriate statement based on the next token
statements_ptr_var Parser::parse_statement() const
{
    auto token = stream_.peek();
    if(!token.has_value())
    {
        auto empty_loc = SourceLocation{0, 0, false};
        reporter_.report_error(empty_loc, "No statement was found.", ErrorType::UNKNOWN);
        synchronize_tokens();
        return builder_.build_stmt_err(empty_loc);
    }

    switch(token.value().type)
    {
    case TokenType::KW_BREAK:
        return builder_.build_break(token.value().loc);
    case TokenType::KW_CONTINUE:
        return builder_.build_continue(token.value().loc);
    // case tokentype::KW_FOR: this isnt implemented yet
    case TokenType::KW_IF:
        return parse_if();
    // case TokenType::KW_STRUCT:
    // This is an incomplete implementation so itll remain disabled for now
    //     return parse_struct();
    case TokenType::KW_RETURN:
        return parse_return();
    case TokenType::KW_WHILE:
        return parse_while();
    case TokenType::TYPE_BOOL:
        return parse_builtin_var(BuiltinType::BOOL);
    case TokenType::TYPE_INT:
        return parse_builtin_var(BuiltinType::INT);
    case TokenType::IDENTIFIER:
        return parse_from_ident();
        break;
    // case TokenType::BRACE_L:
    // scopes arent statements but it might be a constructor? Depends on full struct implementation
    case TokenType::DELIMITER_SEMICOLON:
        {
            stream_.consume();
            return parse_statement();
        }
    default:
        {
            reporter_.report_error(token.value().loc,
                                   "No statement was found.",
                                   ErrorType::UNKNOWN);
            synchronize_tokens();
            return builder_.build_stmt_err(token.value().loc);
        }
    }
}

// Expects tokens: IDENT
// Will continue parsing assuming that those tokens were confirmed
// Will return either a declaration, declaration+assignment, or assignment
statements_ptr_var Parser::parse_from_ident() const
{
    auto token = stream_.peek(1);

    if(!token.has_value())
    {
        auto loc = stream_.peek().value().loc;
        reporter_.report_error(loc, "Unexpected end of input after identifier.", ErrorType::SYNTAX);
        synchronize_tokens();
        return builder_.build_stmt_err(loc);
    }

    switch(token.value().type)
    {
    case TokenType::OP_EQUAL:
        {
        }
    case TokenType::IDENTIFIER:
        return parse_declassign();
    default:
        {
            auto loc = stream_.peek().value().loc;
            reporter_.report_error(loc, "Invalid statement after identifier.", ErrorType::SYNTAX);
            synchronize_tokens();
            return builder_.build_stmt_err(loc);
        }
    }
}

// Expects tokens: IDENT OP_EQUAL
// Will continue parsing assuming that those tokens were confirmed
// Will return an assignment
statements_ptr_var Parser::parse_assign() const
{
    auto name = stream_.consume().value();
    stream_.consume();
    auto expr = parse_expression();
    auto semi = stream_.expect(TokenType::DELIMITER_SEMICOLON);
    if(!semi.has_value())
    {
        reporter_.report_error(name.loc, "Expected ';' after declaration.", ErrorType::SYNTAX);
        synchronize_tokens();
        return builder_.build_stmt_err(name.loc);
    }

    auto ast_name = name.value;

    return builder_.build_assign(name.loc, ast_name, std::move(expr));
}

// IDENT IDENT ...
// Expect tokens: IDENT IDENT
// Will continue parsing assuming that those tokens were confirmed
// Will return either a declaration or a declaration+assignment
statements_ptr_var Parser::parse_declassign() const
{
    auto token = stream_.peek(2);

    if(!token.has_value())
    {
        auto loc = stream_.peek().value().loc;
        reporter_.report_error(loc, "Unexpected end of input after identifier.", ErrorType::SYNTAX);
        synchronize_tokens();
        return builder_.build_stmt_err(loc);
    }

    switch(token.value().type)
    {
    case TokenType::OP_EQUAL:
        {
            auto ident_type = stream_.consume().value();
            auto ident_name = stream_.consume().value();
            stream_.consume();

            auto ast_ident_type = ident_type.value;
            auto ast_ident_name = ident_name.value;
            auto expr = parse_expression();

            auto semi = stream_.expect(TokenType::DELIMITER_SEMICOLON);
            if(!semi.has_value())
            {
                reporter_.report_error(ident_name.loc,
                                       "Expected ';' after declaration and assignment.",
                                       ErrorType::SYNTAX);
                synchronize_tokens();
                return builder_.build_stmt_err(ident_name.loc);
            }

            return builder_.build_declareassign(ident_name.loc,
                                                ast_ident_type,
                                                ast_ident_name,
                                                std::move(expr));
        }
    case TokenType::DELIMITER_SEMICOLON:
        {
            auto type = stream_.consume().value();
            auto name = stream_.consume().value();

            stream_.consume();

            auto ast_type = type.value;
            auto ast_name = name.value;

            return builder_.build_declare(type.loc, ast_type, ast_name);
        }
    default:
        {
            auto loc = stream_.peek().value().loc;
            reporter_.report_error(loc, "Invalid statement after identifier.", ErrorType::SYNTAX);
            synchronize_tokens();
            return builder_.build_stmt_err(loc);
        }
    }
}

// whatever an expression is mhm
expression_ptr_var Parser::parse_expression() const
{
    auto token = stream_.peek();

    if(!token.has_value())
    {
        auto empty_loc = SourceLocation{0, 0, false};
        reporter_.report_error(empty_loc,
                               "Expected expression but found end of input.",
                               ErrorType::SYNTAX);
        synchronize_tokens();
        return builder_.build_expr_err(empty_loc);
    }

    stream_.consume();

    expression_ptr_var lhs;

    switch(token.value().type)
    {
    case TokenType::IDENTIFIER:
        lhs = builder_.build_identifier(token.value().loc, token.value().value);
        break;
    case TokenType::INT_LITERAL:
        lhs = builder_.build_integer(token.value().loc, std::stoi(token.value().value.data()));
        break;
    case TokenType::BOOL_LITERAL:
        lhs = builder_.build_boolean(token.value().loc, string_to_bool(token.value().value));
        break;
    case TokenType::PAREN_R:
        {
            auto expr = parse_expression();
            auto close = stream_.expect(TokenType::PAREN_L);

            if(!close.has_value())
            {
                auto loc = stream_.peek().value().loc;
                reporter_.report_error(loc, "Expected ')' after expression.", ErrorType::SYNTAX);
                synchronize_tokens();
                return builder_.build_expr_err(loc);
            }
            lhs = std::move(expr);
            break;
        }
    case TokenType::OP_NOT:
        lhs = builder_.build_expression(token.value().loc,
                                        parse_expression(),
                                        builder_.build_expr_err(token.value().loc),
                                        token_to_operator(token.value().type));
        break;
    default:
        {
            auto loc = stream_.peek().value().loc;
            reporter_.report_error(loc, "Invalid expression.", ErrorType::SYNTAX);
            synchronize_tokens();
            return builder_.build_expr_err(loc);
        }
    }

    while(true)
    {
        std::optional<Token> op = stream_.peek();
        std::optional<int> prec;
        if(op.has_value())
        {
            auto it = PRECEDENCE.find(op->type);
            if(it != PRECEDENCE.end())
                prec = it->second;
            else
                break;
        }
        else
            break;

        auto [type, val, loc] = stream_.consume().value();
        const int next_prec = prec.value() + 1;
        auto rhs = parse_expression();

        lhs = builder_.build_expression(token.value().loc,
                                        std::move(lhs),
                                        std::move(rhs),
                                        token_to_operator(type));
    }

    return lhs;
}

// Expects tokens: BUILTIN_TYPE IDENT
// Will continue parsing assuming that those tokens were confirmed
// Will return either a declaration or a declaration+assignment
statements_ptr_var Parser::parse_builtin_var(BuiltinType type) const
{
    auto token = stream_.peek(2);

    if(!token.has_value())
    {
        auto loc = stream_.peek().value().loc;
        reporter_.report_error(loc, "Unexpected end of input after identifier.", ErrorType::SYNTAX);
        synchronize_tokens();
        return builder_.build_stmt_err(loc);
    }
    auto builtin_type = stream_.consume().value();
    auto name = stream_.consume().value();

    auto type_ast = builtin_type.value;
    auto ast_name = name.value;

    switch(token.value().type)
    {
    case TokenType::OP_EQUAL:
        {
            stream_.consume();
            auto expr = parse_expression();
            auto semi = stream_.expect(TokenType::DELIMITER_SEMICOLON);
            if(!semi.has_value())
            {
                auto loc = stream_.peek().value().loc;
                reporter_.report_error(loc,
                                       "Expected ';' after declaration and assignment.",
                                       ErrorType::SYNTAX);
                synchronize_tokens();
                return builder_.build_stmt_err(loc);
            }

            return builder_.build_declareassign(builtin_type.loc,
                                                type_ast,
                                                ast_name,
                                                std::move(expr));
        }
    case TokenType::DELIMITER_SEMICOLON:
        {
            stream_.consume();
            return builder_.build_declare(builtin_type.loc, type_ast, ast_name);
        }
    default:
        {
            auto loc = stream_.peek().value().loc;
            reporter_.report_error(loc, "Invalid statement after identifier.", ErrorType::SYNTAX);
            synchronize_tokens();
            return builder_.build_stmt_err(loc);
        }
    }
}

// KW_RETURN ...
// Expects tokens: KW_RETURN
// Will continue parsing assuming that those tokens were confirmed
// Will return a return statement
statements_ptr_var Parser::parse_return() const
{
    auto ret = stream_.consume().value();
    auto expr = parse_expression();
    auto semi = stream_.expect(TokenType::DELIMITER_SEMICOLON);
    if(!semi.has_value())
    {
        auto loc = stream_.peek().value().loc;
        reporter_.report_error(loc, "Expected ';' after return statement.", ErrorType::SYNTAX);
        synchronize_tokens();
        return builder_.build_stmt_err(loc);
    }

    return builder_.build_return(ret.loc, std::move(expr));
}

// Expects tokens: KW_WHILE
// Will continue parsing assuming that those tokens were confirmed
// Will return a while statement
statements_ptr_var Parser::parse_while() const
{
    auto token = stream_.consume().value();
    auto open_paren = stream_.expect(TokenType::PAREN_L);
    if(!open_paren.has_value())
    {
        auto loc = stream_.peek().value().loc;
        reporter_.report_error(loc,
                               "Expected '(' after 'while' on line " +
                                   std::to_string(token.loc.line),
                               ErrorType::SYNTAX);
        synchronize_tokens();
        return builder_.build_stmt_err(loc);
    }

    auto expr = parse_expression();

    auto close_paren = stream_.expect(TokenType::PAREN_R);
    if(!close_paren.has_value())
    {
        auto loc = stream_.peek().value().loc;
        reporter_.report_error(loc,
                               "Expected ')' after expression on line " +
                                   std::to_string(token.loc.line),
                               ErrorType::SYNTAX);
        synchronize_tokens();
        return builder_.build_stmt_err(loc);
    }

    auto scope = parse_scope();

    return builder_.build_while(token.loc, std::move(expr), std::move(scope));
}

// Expects tokens: KW_IF
// Will continue parsing assuming that those tokens were confirmed
// Will return an if statement
statements_ptr_var Parser::parse_if() const
{
    auto token = stream_.consume().value();
    auto open_paren = stream_.expect(TokenType::PAREN_L);
    if(!open_paren.has_value())
    {
        auto loc = stream_.peek().value().loc;
        reporter_.report_error(loc,
                               "Expected '(' after 'if' on line " + std::to_string(token.loc.line),
                               ErrorType::SYNTAX);
        synchronize_tokens();
        return builder_.build_stmt_err(loc);
    }

    auto expr = parse_expression();

    auto close_paren = stream_.expect(TokenType::PAREN_R);
    if(!close_paren.has_value())
    {
        auto loc = stream_.peek().value().loc;
        reporter_.report_error(loc,
                               "Expected ')' after expression on line " +
                                   std::to_string(token.loc.line),
                               ErrorType::SYNTAX);
        synchronize_tokens();
        return builder_.build_stmt_err(loc);
    }

    auto scope = parse_scope();

    auto else_body = parse_else();

    return builder_.build_if(token.loc, std::move(expr), std::move(scope), std::move(else_body));
}

// Does not expect any token
// Will return an optional else statement
std::optional<else_ptr_var> Parser::parse_else() const
{
    auto else_kw = stream_.expect(TokenType::KW_ELSE);
    if(!else_kw.has_value())
        return std::nullopt;

    std::optional<expression_ptr_var> cond = std::nullopt;
    auto next_token = stream_.peek();
    if(next_token.has_value() && next_token.value().type == TokenType::PAREN_L)
    {
        stream_.consume();
        cond = parse_expression();
        auto close_paren = stream_.expect(TokenType::PAREN_R);
        if(!close_paren.has_value())
        {
            auto loc = stream_.peek().value().loc;
            reporter_.report_error(loc,
                                   "Expected ')' after else condition on line " +
                                       std::to_string(else_kw->loc.line),
                                   ErrorType::SYNTAX);
            synchronize_tokens();
            return std::nullopt;
        }
    }

    auto scope = parse_scope();

    return builder_.build_else(else_kw->loc, std::move(cond), std::move(scope));
}

// Expects tokens: BRACE_L
// Will continue parsing assuming that those tokens were confirmed
// Will return a scope statement
scope_err_ptr_var Parser::parse_scope() const
{
    auto open_br = stream_.expect(TokenType::BRACE_L);
    if(!open_br.has_value())
    {
        auto loc = stream_.peek().value().loc;
        reporter_.report_error(loc,
                               "Expected '{' at start of scope on line " + std::to_string(loc.line),
                               ErrorType::SYNTAX);
        synchronize_tokens();
        return builder_.build_stmt_err(loc);
    }

    std::vector<statements_ptr_var> stmts;
    while(stream_.peek().has_value() && stream_.peek().value().type != TokenType::BRACE_R)
    {
        auto stmt = parse_statement();
        stmts.push_back(std::move(stmt));
    }

    auto close_br = stream_.expect(TokenType::BRACE_R);
    if(!close_br.has_value())
    {
        auto loc = stream_.peek().value().loc;
        reporter_.report_error(loc,
                               "Expected '}' at end of scope on line " +
                                   std::to_string(open_br->loc.line),
                               ErrorType::SYNTAX);
        synchronize_tokens();
        return builder_.build_scope(loc, std::move(stmts));
    }

    return builder_.build_scope(open_br->loc, std::move(stmts));
}

// Expects tokens: KW_STRUCT
// Will continue parsing assuming that those tokens were confirmed
// Will return a struct statement
statements_ptr_var Parser::parse_struct() const
{
    auto token = stream_.peek(1);

    if(!token.has_value())
    {
        auto loc = stream_.peek().value().loc;
        reporter_.report_error(loc,
                               "Expected identifier as type name on line " +
                                   std::to_string(loc.line),
                               ErrorType::SYNTAX);
        synchronize_tokens();
        return builder_.build_stmt_err(loc);
    }
    stream_.consume();                          // keyword
    auto type_name = stream_.consume().value(); // struct name

    auto brace_l = stream_.expect(TokenType::BRACE_L);
    if(!brace_l.has_value())
    {
        auto loc = stream_.peek().value().loc;
        reporter_.report_error(loc,
                               "Expected '{' after struct declaration on line " +
                                   std::to_string(loc.line),
                               ErrorType::SYNTAX);
        synchronize_tokens();
        return builder_.build_stmt_err(loc);
    }

    auto next = stream_.peek();
    std::vector<struct_body_var> members;
    while(next.has_value() && next.value().type != TokenType::BRACE_R)
    {
        auto member = struct_helper();
        if(!member.has_value())
        {
            synchronize_tokens();
            reporter_.report_error(next->loc,
                                   "Invalid member declaration in struct on line " +
                                       std::to_string(next->loc.line),
                                   ErrorType::SYNTAX);
            return builder_.build_stmt_err(next->loc);
        }
        members.emplace_back(std::move(member.value()));
        next = stream_.peek();
    }

    auto brace_r = stream_.expect(TokenType::BRACE_R);
    if(!brace_r.has_value())
    {
        auto loc = stream_.peek().value().loc;
        reporter_.report_error(loc,
                               "Expected '}' after struct body on line " + std::to_string(loc.line),
                               ErrorType::SYNTAX);
        synchronize_tokens();
        return builder_.build_stmt_err(loc);
    }

    // register type
    type_registry_.declare_type(type_name.value);

    return builder_.build_struct(token->loc, type_name.value, std::move(members));
}

struct_body_var Parser::parse_struct_declassign() const
{
    auto token = stream_.peek(2);

    if(!token.has_value())
    {
        auto loc = stream_.peek().value().loc;
        reporter_.report_error(loc, "Unexpected end of input after identifier.", ErrorType::SYNTAX);
        synchronize_tokens();
        return builder_.build_stmt_err(loc);
    }

    switch(token.value().type)
    {
    case TokenType::OP_EQUAL:
        {
            auto ident_type = stream_.consume().value();
            auto ident_name = stream_.consume().value();
            stream_.consume();

            auto ast_ident_type = ident_type.value;
            auto ast_ident_name = ident_name.value;
            auto expr = parse_expression();

            auto semi = stream_.expect(TokenType::DELIMITER_SEMICOLON);
            if(!semi.has_value())
            {
                reporter_.report_error(ident_name.loc,
                                       "Expected ';' after declaration and assignment.",
                                       ErrorType::SYNTAX);
                synchronize_tokens();
                return builder_.build_stmt_err(ident_name.loc);
            }

            return builder_.build_declareassign(ident_name.loc,
                                                ast_ident_type,
                                                ast_ident_name,
                                                std::move(expr));
        }
    case TokenType::DELIMITER_SEMICOLON:
        {
            auto type = stream_.consume().value();
            auto name = stream_.consume().value();

            stream_.consume();

            auto ast_type = type.value;
            auto ast_name = name.value;

            return builder_.build_declare(type.loc, ast_type, ast_name);
        }
    default:
        {
            auto loc = stream_.peek().value().loc;
            reporter_.report_error(loc, "Invalid statement after identifier.", ErrorType::SYNTAX);
            synchronize_tokens();
            return builder_.build_stmt_err(loc);
        }
    }
}

// Will parse a single member inside a struct
// Will return either a declaration or a declaration+assignment
// Can return nullopt on error
std::optional<struct_body_var> Parser::struct_helper() const
{
    if(stream_.peek().value().type != TokenType::IDENTIFIER &&
       stream_.peek(1).value().type != TokenType::IDENTIFIER)
    {
        auto loc = stream_.peek().value().loc;
        reporter_.report_error(loc,
                               "Expected member declaration in struct on line " +
                                   std::to_string(loc.line),
                               ErrorType::SYNTAX);
        return std::nullopt;
    }
    return parse_struct_declassign();
}

bool Parser::string_to_bool(const std::string_view& str) const
{
    if(str == "true")
        return true;
    return false;
}

// Error recovery: Skip tokens until we find a reasonable point to resume parsing
void Parser::synchronize_tokens() const
{
    while(true)
    {
        // Peek at the current token
        auto token = stream_.peek(0);
        if(!token.has_value())
        {
            auto loc = SourceLocation{0, 0, false};
            reporter_.report_error(loc,
                                   "Unexpected end of input during error recovery.",
                                   ErrorType::SYNTAX);
        }

        TokenType type = token->type;

        if(type == TokenType::EOF_)
            return; // Reached end of input

        // --- Success Condition (Can Resume) ---
        // If we find a token that can start a new top-level statement, stop skipping.
        if(type == TokenType::KW_IF || type == TokenType::KW_WHILE ||
           type == TokenType::KW_RETURN || type == TokenType::KW_STRUCT)
        {
            return; // We are now positioned safely to parse the next construct
        }

        // --- Cleanup Condition (Reached End of Scope) ---
        // If we find a token that marks the end of a block/statement, stop skipping.
        if(type == TokenType::DELIMITER_SEMICOLON || type == TokenType::BRACE_R)
        {
            stream_.consume(); // Consume the delimiter to move past it
            return;
        }

        // --- Keep Skipping ---
        stream_.consume(); // Discard the current token and move to the next one
    }
}

Operator Parser::token_to_operator(TokenType type) const
{
    auto it = TOKEN_OP.find(type);
    if(it != TOKEN_OP.end())
    {
        return it->second;
    }
    return Operator::UNDEFINED;
}
