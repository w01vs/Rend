#include "constants.hpp"

ConstantOptimizer::ConstantOptimizer(program_ptr&& program)
    : program_(std::move(program)), builder_()
{
}

program_ptr&& ConstantOptimizer::optimize()
{
    for(auto& stmt : program_->stmts)
    {
        std::visit(Overload{[this](scope_ptr& scope) -> void {
                                std::visit(
                                    Overload{[this](std::vector<statements_ptr_var>& vec) -> void {
                                                 for(auto& stmt : vec) { optimize_stmt(stmt); }
                                             },
                                             [this](auto&& stmt) -> void {}},
                                    scope->stmts);
                            },
                            [this](return_ptr& _return) -> void {
                                optimize_return(_return);
                            },
                            [this](if_ptr& _if) -> void {
                                optimize_if(_if);
                            },
                            [this](while_ptr& _while) -> void {
                                optimize_while(_while);
                            },
                            [this](declareassign_ptr& _declareassign) -> void {
                                optimize_declareassign(_declareassign);
                            },
                            [this](assign_ptr& _assign) -> void {
                                optimize_assign(_assign);
                            },
                            [this](auto&& stmt) -> void {}},
                   stmt);
    }

    return std::move(program_);
}

void ConstantOptimizer::optimize_stmt(statements_ptr_var& node)
{
    std::visit(Overload{[this](scope_ptr& scope) -> void {
                            std::visit(Overload{[this](
                                                    std::vector<statements_ptr_var>& vec) -> void {
                                                    for(auto& stmt : vec) { optimize_stmt(stmt); }
                                                },
                                                [this](auto&& stmt) -> void {}},
                                       scope->stmts);
                        },
                        [this](return_ptr& _return) -> void {
                            optimize_return(_return);
                        },
                        [this](if_ptr& _if) -> void {
                            optimize_if(_if);
                        },
                        [this](while_ptr& _while) -> void {
                            optimize_while(_while);
                        },
                        [this](declareassign_ptr& _declareassign) -> void {
                            optimize_declareassign(_declareassign);
                        },
                        [this](assign_ptr& _assign) -> void {
                            optimize_assign(_assign);
                        },
                        [this](auto&& stmt) -> void {}},
               node);
}

void ConstantOptimizer::optimize_return(return_ptr& _return)
{
    auto expr = fold_constants(_return->val);
    if(expr.has_value())
        _return->val = std::move(expr.value());
}

void ConstantOptimizer::optimize_if(if_ptr& _if)
{
    auto expr = fold_constants(_if->condition);
    if(expr.has_value())
        _if->condition = std::move(expr.value());
    std::visit(Overload{[this](scope_ptr& scope) -> void {
                            std::visit(Overload{[this](
                                                    std::vector<statements_ptr_var>& vec) -> void {
                                                    for(auto& stmt : vec) { optimize_stmt(stmt); }
                                                },
                                                [this](auto&& stmt) -> void {}},
                                       scope->stmts);
                        },
                        [this](auto&& scope) -> void {}},
               _if->scope);
    if(_if->else_clause.has_value())
    {
        auto& else_clause = _if->else_clause.value();
        std::visit(Overload{[this](else_ptr& _else) {
                       if(_else->condition.has_value())
                       {
                           auto expr = fold_constants(_else->condition.value());
                           if(expr.has_value())
                               _else->condition = std::move(expr.value());
                       }
                       std::visit(Overload{[this](scope_ptr&& scope) -> void {
                                               std::visit(
                                                   Overload{[this](std::vector<statements_ptr_var>&
                                                                       vec) -> void {
                                                                for(auto& stmt : vec)
                                                                {
                                                                    optimize_stmt(stmt);
                                                                }
                                                            },
                                                            [this](auto&& stmt) -> void {}},
                                                   scope->stmts);
                                           },
                                           [this](auto&& scope) -> void {}},
                                  _else->scope);
                   }},
                   else_clause);
    }
}

void ConstantOptimizer::optimize_while(while_ptr& _while)
{
    auto expr = fold_constants(_while->condition);
    if(expr.has_value())
        _while->condition = std::move(expr.value());
    std::visit(Overload{[this](scope_ptr&& scope) -> void {
                            std::visit(Overload{[this](
                                                    std::vector<statements_ptr_var>&& vec) -> void {
                                                    for(auto& stmt : vec) { optimize_stmt(stmt); }
                                                },
                                                [this](auto&& stmt) -> void {}},
                                       scope->stmts);
                        },
                        [this](auto&& scope) -> void {}},
               _while->scope);
}

void ConstantOptimizer::optimize_declareassign(declareassign_ptr& _declareassign)
{
    auto expr = fold_constants(_declareassign->expr);
    if(expr.has_value())
        _declareassign->expr = std::move(expr.value());
}

void ConstantOptimizer::optimize_assign(assign_ptr& _assign)
{
    auto expr = fold_constants(_assign->expr);
    if(expr.has_value())
        _assign->expr = std::move(expr.value());
}

std::optional<expression_ptr_var&&> ConstantOptimizer::fold_constants(expression_ptr_var& node)
{

}
