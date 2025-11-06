#include "hir_generator.hpp"

HIRGen::HIRGen(program_ptr& program, std::map<std::string_view, Var, std::less<>>& symbols)
    : program_(program), symbols_(symbols)
{
}

program_ptr&& HIRGen::analyze()
{
    for(auto& stmt : program_->stmts) { analyze_stmt(stmt); }
    return std::move(program_);
}

void HIRGen::analyze_stmt(statements_ptr_var& node)
{
    std::visit(Overload{[this](scope_ptr& scope)
                        {
                            std::visit(Overload{[this](std::vector<statements_ptr_var>& vec)
                                                {
                                                    for(auto& stmt : vec) { analyze_stmt(stmt); }
                                                },
                                                [this](stmt_err_ptr& err) {},
                                                [this](auto&&) {}},
                                       scope->stmts);
                        },
                        [this](
                            break_ptr& _break) { // hir_stmt_.push_back(HIRJump{current_label_++});
                        },
                        [this](continue_ptr& _continue) {},
                        [this](return_ptr& _return) {},
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

void HIRGen::analyze_else_var(else_ptr_var& node)
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

void HIRGen::analyze_scope_var(scope_err_ptr_var& node)
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
