#include "semantics.hpp"

#define REPORT_ERROR(loc, msg, type) reporter_.report_error(loc, msg, type, __FILE__, __LINE__);

type::TypeRegistry& SemanticAnalyzer::typeregistry_ = type::TypeRegistry::instance();

const std::unordered_map<OperatorMatrixIndex, OperatorResult> SemanticAnalyzer::OPERATOR_MATRIX = {
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
    {{typeregistry_._int_(), Operator::GREATER, typeregistry_._int_()}, {typeregistry_._bool_()}},
    {{typeregistry_._int_(), Operator::LESSEQ, typeregistry_._int_()}, {typeregistry_._bool_()}},
    {{typeregistry_._int_(), Operator::GREATEREQ, typeregistry_._int_()}, {typeregistry_._bool_()}},
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
    scope_stack_.push_back(scope_id_++);
}

program_ptr&& SemanticAnalyzer::analyze()
{
    for(auto& stmt : program_->stmts) { analyze_stmt(stmt); }
    return std::move(program_);
}

void SemanticAnalyzer::analyze_stmt(statements_ptr_var& node)
{
    std::visit(Overload{[this](scope_ptr& scope) {
                            std::visit(
                                Overload{[this](std::vector<statements_ptr_var>& vec) {
                                             scope_stack_.push_back(scope_id_++);
                                             for(auto& stmt : vec) { analyze_stmt(stmt); }
                                             scope_stack_.pop_back();
                                         },
                                         [this](stmt_err_ptr& err) {
                                             REPORT_ERROR(err->loc,
                                                          "Failed parsing statement in scope",
                                                          ErrorType::SEMANTIC);
                                         },
                                         [this](auto&&) {
                                             SourceLocation loc{.valid = false};
                                             REPORT_ERROR(loc,
                                                          "Be scared: this should never happen!",
                                                          ErrorType::UNKNOWN);
                                         }},
                                scope->stmts);
                        },
                        [this](break_ptr& _break) {
                            if(loop_depth_ == 0)
                                REPORT_ERROR(_break->loc,
                                             "Break statement not within loop",
                                             ErrorType::SEMANTIC);
                        },
                        [this](continue_ptr& _continue) {
                            if(loop_depth_ == 0)
                                REPORT_ERROR(_continue->loc,
                                             "Continue statement not within loop",
                                             ErrorType::SEMANTIC);
                        },
                        [this](return_ptr& _return) {
                            auto type = _typeof_(_return->val);
                            auto _int = typeregistry_._int_();
                            if(type != _int)
                                REPORT_ERROR(_return->loc,
                                             "Return type mismatch, expected int",
                                             ErrorType::SEMANTIC);
                        },
                        [this](else_ptr& _else) {
                            if(_else->condition.has_value())
                            {
                                auto& cond = _else->condition.value();
                                auto cond_type = _typeof_(cond);
                                if(cond_type == typeregistry_._undefined_())
                                    return;
                                if(cond_type != typeregistry_._bool_())
                                    REPORT_ERROR(_else->loc,
                                                 "Else if condition must be of type bool",
                                                 ErrorType::SEMANTIC);
                            }

                            analyze_scope_var(_else->scope);
                        },
                        [this](if_ptr& _if) {
                            auto cond_type = _typeof_(_if->condition);
                            if(cond_type == typeregistry_._undefined_())
                                return;
                            if(cond_type != typeregistry_._bool_())
                                REPORT_ERROR(_if->loc,
                                             "If condition must be of type bool",
                                             ErrorType::SEMANTIC);

                            analyze_scope_var(_if->scope);

                            if(_if->else_clause.has_value())
                                analyze_else_var(_if->else_clause.value());
                        },
                        [this](while_ptr& _while) {
                            loop_depth_++;
                            auto cond_type = _typeof_(_while->condition);
                            if(cond_type == typeregistry_._undefined_())
                                return;
                            if(cond_type != typeregistry_._bool_())
                                REPORT_ERROR(_while->loc,
                                             "While condition must be of type bool",
                                             ErrorType::SEMANTIC);
                            analyze_scope_var(_while->scope);
                            loop_depth_--;
                        },
                        [](struct_ptr& _struct) {

                        },
                        [this](declareassign_ptr& declassign) {
                            declassign->ident->scope_id = scope_stack_.back();
                            auto type = _typeof_(declassign->expr);
                            if(type == typeregistry_._undefined_())
                                return;
                            auto declared_type = typeregistry_.find_type(declassign->type_name);
                            if(!declare_variable(declassign->ident, type))
                                REPORT_ERROR(declassign->loc,
                                             "This variable has already been defined",
                                             ErrorType::SEMANTIC);
                            if(declared_type == typeregistry_._undefined_())
                            {
                                REPORT_ERROR(declassign->loc,
                                             "Undefined type in declaration",
                                             ErrorType::SEMANTIC);
                                return;
                            }
                            if(type != declared_type)
                                REPORT_ERROR(declassign->loc,
                                             "Type mismatch in declaration",
                                             ErrorType::SEMANTIC);
                            declassign->type = type;
                        },
                        [this](declare_ptr& declare) {
                            declare->ident->scope_id = scope_stack_.back();
                            auto type = typeregistry_.find_type(declare->type_name);
                            if(!declare_variable(declare->ident, type))
                                REPORT_ERROR(declare->loc,
                                             "This variable has already been defined",
                                             ErrorType::SEMANTIC);
                            declare->type = type;
                        },
                        [this](assign_ptr& assign) {
                            assign->ident->scope_id = scope_stack_.back();
                            auto var_type = find_variable_type(assign->ident);
                            if(var_type == typeregistry_._undefined_())
                                return;
                            auto expr_type = _typeof_(assign->expr);
                            if(expr_type == typeregistry_._undefined_())
                                return;
                            if(var_type != expr_type)
                                REPORT_ERROR(assign->loc,
                                             "Type mismatch in assignment",
                                             ErrorType::SEMANTIC);
                        },
                        [this](stmt_err_ptr& err) {
                            REPORT_ERROR(err->loc, "Failed parsing statement", ErrorType::SEMANTIC);
                        },
                        [this](auto&&) {
                            SourceLocation loc{.valid = false};
                            REPORT_ERROR(loc,
                                         "Analyzing failed: this node has no analysis implemented.",
                                         ErrorType::UNKNOWN);
                        }},
               node);
}

void SemanticAnalyzer::analyze_else_var(else_ptr_var& node)
{
    std::visit(Overload{[this](else_ptr& _else) {
                            if(_else->condition.has_value())
                            {
                                auto& cond = _else->condition.value();
                                auto cond_type = _typeof_(cond);
                                if(cond_type == typeregistry_._undefined_())
                                    return;
                                if(cond_type != typeregistry_._bool_())
                                    REPORT_ERROR(_else->loc,
                                                 "Else if condition must be of type bool",
                                                 ErrorType::SEMANTIC);
                            }

                            analyze_scope_var(_else->scope);
                        },
                        [this](stmt_err_ptr& err) {
                            REPORT_ERROR(err->loc,
                                         "Failed parsing statement in else clause",
                                         ErrorType::SEMANTIC);
                        },
                        [this](auto&&) {
                            SourceLocation loc{.valid = false};
                            REPORT_ERROR(loc,
                                         "Be scared: this should never happen!",
                                         ErrorType::UNKNOWN);
                        }},
               node);
}

void SemanticAnalyzer::analyze_scope_var(scope_err_ptr_var& node)
{
    std::visit(
        Overload{[this](scope_ptr& scope) {
                     std::visit(Overload{[this](std::vector<statements_ptr_var>& vec) {
                                             scope_stack_.push_back(scope_id_++);
                                             for(auto& stmt : vec) { analyze_stmt(stmt); }
                                             scope_stack_.pop_back();
                                         },
                                         [this](stmt_err_ptr& err) {
                                             REPORT_ERROR(err->loc,
                                                          "Failed parsing statement in scope",
                                                          ErrorType::SEMANTIC);
                                         },
                                         [this](auto&&) {
                                             SourceLocation loc{.valid = false};
                                             REPORT_ERROR(loc,
                                                          "Be scared: this should never happen!",
                                                          ErrorType::UNKNOWN);
                                         }},
                                scope->stmts);
                 },
                 [this](stmt_err_ptr& err) {
                     REPORT_ERROR(err->loc,
                                  "Failed parsing statement in while loop",
                                  ErrorType::SEMANTIC);
                 },
                 [this](auto&&) {
                     SourceLocation loc{.valid = false};
                     REPORT_ERROR(loc, "Be scared: this should never happen!", ErrorType::UNKNOWN);
                 }},
        node);
}

std::shared_ptr<type::BuiltinType> SemanticAnalyzer::_typeof_(expression_ptr_var& node) const
{
    return std::visit(Overload{[](integer_ptr& integer) -> std::shared_ptr<type::BuiltinType> {
                                   return typeregistry_._int_();
                               },
                               [](boolean_ptr& boolean) -> std::shared_ptr<type::BuiltinType> {
                                   return typeregistry_._bool_();
                               },
                               [this](identifier_ptr& ident) -> std::shared_ptr<type::BuiltinType> {
                                   std::shared_ptr<type::BuiltinType> var_type =
                                       find_variable_type(ident);
                                   if(var_type == typeregistry_._undefined_())
                                   {
                                       REPORT_ERROR(ident->loc,
                                                    "Use of undeclared variable '" +
                                                        std::string(ident->name) + "'",
                                                    ErrorType::SEMANTIC);
                                   }
                                   return var_type;
                               },
                               [this](expression_ptr& expr) -> std::shared_ptr<type::BuiltinType> {
                                   auto rhs = _typeof_(expr->rhs);
                                   if(rhs == typeregistry_._undefined_())
                                       return typeregistry_._undefined_();
                                   auto lhs = _typeof_(expr->lhs);
                                   if(lhs == typeregistry_._undefined_())
                                       return typeregistry_._undefined_();

                                   OperatorMatrixIndex idx = {rhs, expr->op, lhs};
                                   auto it = OPERATOR_MATRIX.find(idx);
                                   if(it == OPERATOR_MATRIX.end())
                                       return typeregistry_._undefined_();

                                   return it->second.result;
                               },
                               [](expr_err_ptr& err) -> std::shared_ptr<type::BuiltinType> {
                                   return typeregistry_._undefined_();
                               },
                               [](auto&&) -> std::shared_ptr<type::BuiltinType> {
                                   return typeregistry_._undefined_();
                               }},
                      node);
}

// Will return true when variable is declared, false if it already exists
bool SemanticAnalyzer::declare_variable(identifier_ptr& ident,
                                        std::shared_ptr<type::BuiltinType> type)
{
    auto it = variables_.find({ident->name, scope_stack_.back()});
    if(it != variables_.end())
        return false;
    variables_[{ident->name, scope_stack_.back()}] = Var{ident->name, type};
    ident->scope_id = scope_stack_.back();
    return true;
}

std::shared_ptr<type::BuiltinType> SemanticAnalyzer::find_variable_type(identifier_ptr& ident) const
{
    
    size_t i = scope_stack_.size() - 1;
    auto it = variables_.find({ident->name, scope_stack_.at(i)});
    while (it == variables_.end() && i > 0) {
        i--;
        it = variables_.find({ident->name, scope_stack_.at(i)});
    }
    if(it == variables_.end())
        return typeregistry_._undefined_();
    ident->scope_id = scope_stack_.at(i);
    return it->second.type;
}
