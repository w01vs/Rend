#include "semantics.hpp"

using semantics::OperatorMatrixIndex;

type::TypeRegistry& SemanticAnalyzer::typeregistry_ = type::TypeRegistry::instance();

const std::unordered_map<OperatorMatrixIndex, semantics::OperatorResult>
    SemanticAnalyzer::OPERATOR_MATRIX = {
        // INT RESULTS
        {{typeregistry_._int_(), Operator::ADD, typeregistry_._int_()}, {typeregistry_._int_()}},
        {{typeregistry_._int_(), Operator::SUB, typeregistry_._int_()}, {typeregistry_._int_()}},
        {{typeregistry_._int_(), Operator::MUL, typeregistry_._int_()}, {typeregistry_._int_()}},
        {{typeregistry_._int_(), Operator::DIV, typeregistry_._int_()}, {typeregistry_._int_()}},
        {{typeregistry_._int_(), Operator::MOD, typeregistry_._int_()}, {typeregistry_._int_()}},
        {{typeregistry_._int_(), Operator::LSH, typeregistry_._int_()}, {typeregistry_._int_()}},
        {{typeregistry_._int_(), Operator::RSH, typeregistry_._int_()}, {typeregistry_._int_()}},
        {{typeregistry_._int_(), Operator::BAND, typeregistry_._int_()}, {typeregistry_._int_()}},
        {{typeregistry_._int_(), Operator::XOR, typeregistry_._int_()}, {typeregistry_._int_()}},
        {{typeregistry_._int_(), Operator::BOR, typeregistry_._int_()}, {typeregistry_._int_()}},
        // BOOL RESULTS
        {{typeregistry_._int_(), Operator::LESS, typeregistry_._int_()}, {typeregistry_._bool_()}},
        {{typeregistry_._int_(), Operator::GREATER, typeregistry_._int_()},
         {typeregistry_._bool_()}},
        {{typeregistry_._int_(), Operator::LESSEQ, typeregistry_._int_()},
         {typeregistry_._bool_()}},
        {{typeregistry_._int_(), Operator::GREATEREQ, typeregistry_._int_()},
         {typeregistry_._bool_()}},
        {{typeregistry_._int_(), Operator::EQ, typeregistry_._int_()}, {typeregistry_._bool_()}},
        {{typeregistry_._int_(), Operator::NEQ, typeregistry_._int_()}, {typeregistry_._bool_()}},
        {{typeregistry_._bool_(), Operator::AND, typeregistry_._bool_()}, {typeregistry_._bool_()}},
        {{typeregistry_._bool_(), Operator::OR, typeregistry_._bool_()}, {typeregistry_._bool_()}},
        {{typeregistry_._bool_(), Operator::XOR, typeregistry_._bool_()}, {typeregistry_._bool_()}},
        {{typeregistry_._bool_(), Operator::NOT, typeregistry_._bool_()}, {typeregistry_._bool_()}},
};

SemanticAnalyzer::SemanticAnalyzer(program_ptr&& program, ErrorReporter& reporter)
    : program_(std::move(program)), reporter_(reporter)
{
}

program_ptr&& SemanticAnalyzer::analyze()
{
    for(auto& stmt : program_->stmts) { analyze_stmt(stmt); }
    return std::move(program_);
}

void SemanticAnalyzer::analyze_stmt(statements_ptr_var& node)
{
    std::visit(Overload{[this](scope_ptr& scope)
                        {
                            std::visit(Overload{[this](std::vector<statements_ptr_var>& vec)
                                                {
                                                    for(auto& stmt : vec) { analyze_stmt(stmt); }
                                                },
                                                [this](stmt_err_ptr& err) {
                                                    reporter_.report_error(
                                                        err->loc,
                                                        "Failed parsing statement in scope",
                                                        ErrorType::SEMANTIC);
                                                },
                                                [this](auto&&)
                                                {
                                                    SourceLocation loc{};
                                                    reporter_.report_error(
                                                        loc,
                                                        "Be scared: this should never happen!",
                                                        ErrorType::UNKNOWN);
                                                }},
                                       scope->stmts);
                        },
                        [this](break_ptr& _break)
                        {
                            if(loop_depth_ == 0)
                                reporter_.report_error(_break->loc,
                                                       "Break statement not within loop",
                                                       ErrorType::SEMANTIC);
                        },
                        [this](continue_ptr& _continue)
                        {
                            if(loop_depth_ == 0)
                                reporter_.report_error(_continue->loc,
                                                       "Continue statement not within loop",
                                                       ErrorType::SEMANTIC);
                        },
                        [this](return_ptr& _return)
                        {
                            auto type = _typeof_(_return->val);
                            auto _int = typeregistry_._int_();
                            if(type != _int)
                                reporter_.report_error(_return->loc,
                                                       "Return type mismatch, expected int",
                                                       ErrorType::SEMANTIC);
                        },
                        [this](else_ptr& _else)
                        {
                            if(_else->condition.has_value())
                            {
                                auto& cond = _else->condition.value();
                                auto cond_type = _typeof_(cond);
                                if(cond_type != typeregistry_._bool_())
                                    reporter_.report_error(_else->loc,
                                                           "Else if condition must be of type bool",
                                                           ErrorType::SEMANTIC);
                            }

                            analyze_scope_var(_else->scope);
                        },
                        [this](if_ptr& _if)
                        {
                            auto cond_type = _typeof_(_if->condition);
                            if(cond_type != typeregistry_._bool_())
                                reporter_.report_error(_if->loc,
                                                       "If condition must be of type bool",
                                                       ErrorType::SEMANTIC);

                            analyze_scope_var(_if->scope);

                            if(_if->else_clause.has_value())
                                analyze_else_var(_if->else_clause.value());
                        },
                        [this](while_ptr& _while)
                        {
                            loop_depth_++;
                            auto cond_type = _typeof_(_while->condition);
                            if(cond_type != typeregistry_._bool_())
                                reporter_.report_error(_while->loc,
                                                       "While condition must be of type bool",
                                                       ErrorType::SEMANTIC);
                            analyze_scope_var(_while->scope);
                            loop_depth_--;
                        },
                        [](struct_ptr& _struct) {

                        },
                        [this](declareassign_ptr& declassign)
                        {
                            auto type = _typeof_(declassign->expr);
                            auto declared_type = typeregistry_.find_type(declassign->type_name);
                            if(type == typeregistry_._undefined_())
                                reporter_.report_error(declassign->loc,
                                                       "Undefined type in declaration",
                                                       ErrorType::SEMANTIC);
                            if(declared_type == typeregistry_._undefined_())
                                reporter_.report_error(declassign->loc,
                                                       "Undefined declared type in declaration",
                                                       ErrorType::SEMANTIC);
                            if(type != declared_type)
                                reporter_.report_error(declassign->loc,
                                                       "Type mismatch in declaration",
                                                       ErrorType::SEMANTIC);
                            if(!declare_variable(declassign->name, type))
                                reporter_.report_error(declassign->loc,
                                                       "This variable has already been defined",
                                                       ErrorType::SEMANTIC);
                            declassign->type = type;
                        },
                        [this](declare_ptr& declare)
                        {
                            auto type = typeregistry_.find_type(declare->type_name);
                            if(type == typeregistry_._undefined_())
                                reporter_.report_error(declare->loc,
                                                       "Undefined type in declaration",
                                                       ErrorType::SEMANTIC);
                            if(!declare_variable(declare->name, type))
                                reporter_.report_error(declare->loc,
                                                       "This variable has already been defined",
                                                       ErrorType::SEMANTIC);
                            declare->type = type;
                        },
                        [this](assign_ptr& assign)
                        {
                            auto var_type = find_variable_type(assign->name);
                            auto expr_type = _typeof_(assign->expr);
                            if(var_type != expr_type)
                                reporter_.report_error(assign->loc,
                                                       "Type mismatch in assignment",
                                                       ErrorType::SEMANTIC);
                            if(expr_type == typeregistry_._undefined_())
                                reporter_.report_error(assign->loc,
                                                       "Undefined type in assignment",
                                                       ErrorType::SEMANTIC);
                        },
                        [this](stmt_err_ptr& err) {
                            reporter_.report_error(err->loc,
                                                   "Failed parsing statement",
                                                   ErrorType::SEMANTIC);
                        },
                        [this](auto&&)
                        {
                            SourceLocation loc{};
                            reporter_.report_error(loc,
                                                   "Be scared: this should never happen!",
                                                   ErrorType::UNKNOWN);
                        }},
               node);
}

void SemanticAnalyzer::analyze_else_var(else_ptr_var& node)
{
    std::visit(Overload{[this](else_ptr& _else)
                        {
                            if(_else->condition.has_value())
                            {
                                auto& cond = _else->condition.value();
                                auto cond_type = _typeof_(cond);
                                if(cond_type != typeregistry_._bool_())
                                    reporter_.report_error(_else->loc,
                                                           "Else if condition must be of type bool",
                                                           ErrorType::SEMANTIC);
                            }

                            analyze_scope_var(_else->scope);
                        },
                        [this](stmt_err_ptr& err)
                        {
                            reporter_.report_error(err->loc,
                                                   "Failed parsing statement in else clause",
                                                   ErrorType::SEMANTIC);
                        },
                        [this](auto&&)
                        {
                            SourceLocation loc{};
                            reporter_.report_error(loc,
                                                   "Be scared: this should never happen!",
                                                   ErrorType::UNKNOWN);
                        }},
               node);
}

void SemanticAnalyzer::analyze_scope_var(scope_err_ptr_var& node)
{
    std::visit(Overload{[this](scope_ptr& scope)
                        {
                            std::visit(Overload{[this](std::vector<statements_ptr_var>& vec)
                                                {
                                                    for(auto& stmt : vec) { analyze_stmt(stmt); }
                                                },
                                                [this](stmt_err_ptr& err) {
                                                    reporter_.report_error(
                                                        err->loc,
                                                        "Failed parsing statement in scope",
                                                        ErrorType::SEMANTIC);
                                                },
                                                [this](auto&&)
                                                {
                                                    SourceLocation loc{};
                                                    reporter_.report_error(
                                                        loc,
                                                        "Be scared: this should never happen!",
                                                        ErrorType::UNKNOWN);
                                                }},
                                       scope->stmts);
                        },
                        [this](stmt_err_ptr& err)
                        {
                            reporter_.report_error(err->loc,
                                                   "Failed parsing statement in while loop",
                                                   ErrorType::SEMANTIC);
                        },
                        [this](auto&&)
                        {
                            SourceLocation loc{};
                            reporter_.report_error(loc,
                                                   "Be scared: this should never happen!",
                                                   ErrorType::UNKNOWN);
                        }},
               node);
}

std::shared_ptr<type::BuiltinType> SemanticAnalyzer::_typeof_(expression_ptr_var& node) const
{
    return std::visit(Overload{[](integer_ptr& integer) -> std::shared_ptr<type::BuiltinType>
                               { return typeregistry_._int_(); },
                               [](boolean_ptr& boolean) -> std::shared_ptr<type::BuiltinType>
                               { return typeregistry_._bool_(); },
                               [this](identifier_ptr& ident) -> std::shared_ptr<type::BuiltinType>
                               { return find_variable_type(ident->name); },
                               [this](expression_ptr& expr) -> std::shared_ptr<type::BuiltinType>
                               {
                                   auto rhs = _typeof_(expr->rhs);
                                   auto lhs = _typeof_(expr->lhs);

                                   OperatorMatrixIndex idx = {rhs, expr->op, lhs};
                                   auto it = OPERATOR_MATRIX.find(idx);
                                   if(it == OPERATOR_MATRIX.end())
                                       return typeregistry_._undefined_();

                                   return it->second.result;
                               },
                               [](expr_err_ptr& err) -> std::shared_ptr<type::BuiltinType>
                               { return typeregistry_._undefined_(); },
                               [](auto&&) -> std::shared_ptr<type::BuiltinType>
                               { return typeregistry_._undefined_(); }},
                      node);
}

// Will return true when variable is declared, false if it already exists
bool SemanticAnalyzer::declare_variable(std::string_view name,
                                        std::shared_ptr<type::BuiltinType> type)
{
    auto it = variables_.find(name);
    if(it == variables_.end())
        return false;
    variables_[name] = Var{name, type};
    return true;
}

std::shared_ptr<type::BuiltinType> SemanticAnalyzer::find_variable_type(std::string_view name) const
{
    auto it = variables_.find(name);
    if(it == variables_.end())
        return typeregistry_._undefined_();
    return it->second.type;
}
