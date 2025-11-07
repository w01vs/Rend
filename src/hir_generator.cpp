#include "hir_generator.hpp"

HIRGen::HIRGen(program_ptr& program, std::map<std::string_view, Var, std::less<>>& symbols)
    : program_(program), symbols_(symbols), current_label_(0), current_register_(0)
{
}

program_ptr&& HIRGen::analyze()
{
    for(auto& stmt : program_->stmts) { analyze_stmt(stmt); }
    return std::move(program_);
}

void HIRGen::fold_flat_constants(std::vector<FlattenedExpr>& operands, Operator op) const
{
    switch(op)
    {
    case Operator::ADD:
        {
            int folded = 0;
            for(auto& flattened : operands)
            {
                if(std::holds_alternative<int>(flattened))
                {
                    int constant = std::get<int>(flattened);
                    folded += constant;
                }
            }
        }
    case Operator::MUL:
        {
            int folded = 1;
            for(auto& flattened : operands)
            {
                if(std::holds_alternative<int>(flattened))
                {
                    int constant = std::get<int>(flattened);
                    folded *= constant;
                }
            }
        }
    default:
        break;
    }
}

void HIRGen::collect_operands(expression_ptr_var& expr, std::vector<FlattenedExpr>& operands,
                              Operator op) const
{
    std::visit(Overload{[&operands](integer_ptr& integer) { operands.push_back(integer->value); },
                        [&operands](boolean_ptr& boolean)
                        { operands.push_back(static_cast<int>(boolean->value)); },
                        [&operands](identifier_ptr& ident) { operands.push_back(ident->name); },
                        [&operands, &op, this](expression_ptr& expr)
                        {
                            if(expr->op == op)
                            {
                                collect_operands(expr->lhs, operands, op);
                                collect_operands(expr->rhs, operands, op);
                                return;
                            }

                            operands.push_back(std::move(expr));
                        }},
               expr);
}

HIRExprFactor HIRGen::unfold_expression(expression_ptr_var& expr)
{
    return std::visit(Overload{[](integer_ptr& integer) -> HIRExprFactor
                               { return VirtualRegisterID(integer->value); },
                               [](boolean_ptr& boolean) -> HIRExprFactor
                               { return static_cast<int>(boolean->value); },
                               [this](identifier_ptr& ident) -> HIRExprFactor
                               { return ident->name; },
                               [this](expression_ptr& expr) -> HIRExprFactor
                               {
                                   Operator op = expr->op;
                                   // if op is associative and commutative
                                   std::vector<FlattenedExpr> operands{};
                                   collect_operands(expr->lhs, operands, op);
                                   collect_operands(expr->rhs, operands, op);

                                   fold_flat_constants(operands, op);

                                   // if op is not associative and commutative

                                   return 1;
                               },
                               [](auto&&) -> HIRExprFactor { return 1; }},
                      expr);
}

void HIRGen::analyze_stmt(statements_ptr_var& node)
{
    std::visit(Overload{[this](scope_ptr& scope) {},
                        [this](break_ptr& _break) {},
                        [this](continue_ptr& _continue) {},
                        [this](return_ptr& _return) {},
                        [this](else_ptr& _else) {},
                        [this](if_ptr& _if) {},
                        [this](while_ptr& _while) {},
                        [](struct_ptr& _struct) {},
                        [this](declareassign_ptr& declassign) {},
                        [this](declare_ptr& declare) {},
                        [this](assign_ptr& assign) {},
                        [this](auto&&) {}},
               node);
}

void HIRGen::analyze_else_var(else_ptr_var& node)
{
    std::visit(Overload{[this](else_ptr& _else) {}, [this](auto&&) {}}, node);
}

void HIRGen::analyze_scope_var(scope_err_ptr_var& node)
{
    std::visit(Overload{[this](scope_ptr& scope)
                        {
                            std::visit(Overload{[this](std::vector<statements_ptr_var>& vec) {},
                                                [this](auto&&) {}},
                                       scope->stmts);
                        },
                        [this](auto&&) {}},
               node);
}
