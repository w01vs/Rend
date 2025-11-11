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
    std::vector<FlattenedExpr> flattened_operands{};
    flattened_operands.reserve(operands.size());
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
                else
                {
                    flattened_operands.emplace_back(std::move(flattened));
                }
            }
            flattened_operands.emplace_back(folded);
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
                else
                {
                    flattened_operands.emplace_back(std::move(flattened));
                }
            }
            flattened_operands.emplace_back(folded);
        }
    case Operator::BAND:
        {
            int folded = -1;
            for(auto& flattened : operands)
            {
                if(std::holds_alternative<int>(flattened))
                {
                    int constant = std::get<int>(flattened);
                    folded &= constant;
                }
                else
                {
                    flattened_operands.emplace_back(std::move(flattened));
                }
            }
            flattened_operands.emplace_back(folded);
        }
    case Operator::AND:
        {
            bool folded;
            for(auto& flattened : operands)
            {
                if(std::holds_alternative<int>(flattened))
                {
                    int constant = std::get<int>(flattened);
                    folded = constant == 1 ? true : false;
                    if(!folded)
                        break;
                }
                else
                {
                    flattened_operands.emplace_back(std::move(flattened));
                }
            }
            if(!folded)
            {
                flattened_operands.clear();
                flattened_operands.emplace_back(0);
                return;
            }
            flattened_operands.emplace_back(1);
        }
    case Operator::BOR:
        {
            int folded = 0;
            for(auto& flattened : operands)
            {
                if(std::holds_alternative<int>(flattened))
                {
                    int constant = std::get<int>(flattened);
                    folded |= constant;
                }
                else
                {
                    flattened_operands.emplace_back(std::move(flattened));
                }
            }
            flattened_operands.emplace_back(folded);
        }
    case Operator::OR:
        {
            bool folded;
            for(auto& flattened : operands)
            {
                if(std::holds_alternative<int>(flattened))
                {
                    int constant = std::get<int>(flattened);
                    folded = constant == 1 ? true : false;
                    if(folded)
                        break;
                }
                else
                {
                    flattened_operands.emplace_back(std::move(flattened));
                }
            }
            flattened_operands.emplace_back(folded ? 1 : 0);
        }
    case Operator::XOR:
        {
            int folded = 0;
            for(auto& flattened : operands)
            {
                if(std::holds_alternative<int>(flattened))
                {
                    int constant = std::get<int>(flattened);
                    folded ^= constant;
                }
                else
                {
                    flattened_operands.emplace_back(std::move(flattened));
                }
            }
            flattened_operands.emplace_back(folded);
        }
    default:
        break;
    }
    operands = std::move(flattened_operands);
}

void HIRGen::collect_operands(expression_ptr_var& expr, std::vector<FlattenedExpr>& operands,
                              Operator op) const
{
    std::visit(Overload{[&operands](integer_ptr& integer) {
                            operands.emplace_back(integer->value);
                        },
                        [&operands](boolean_ptr& boolean) {
                            operands.emplace_back(static_cast<int>(boolean->value));
                        },
                        [&operands](identifier_ptr& ident) { operands.emplace_back(ident->name); },
                        [&operands, &op, this](expression_ptr& expr) {
                            if(expr->op == op)
                            {
                                collect_operands(expr->lhs, operands, op);
                                collect_operands(expr->rhs, operands, op);
                                return;
                            }

                            operands.emplace_back(std::move(expr));
                        },
                        [](auto&&) {}},
               expr);
}

HIRExprFactor HIRGen::unfold_expression(expression_ptr_var& expr)
{
    return std::visit(
        Overload{[](integer_ptr& integer) -> HIRExprFactor { return integer->value; },
                 [](boolean_ptr& boolean) -> HIRExprFactor {
                     return static_cast<int>(boolean->value);
                 },
                 [this](identifier_ptr& ident) -> HIRExprFactor {
                     VirtualRegisterID vreg{current_register_++};
                     hir_stmt_.emplace_back(std::in_place_type<HIRLoad>, vreg, ident->name);
                     return vreg;
                 },
                 [this](expression_ptr& expr) -> HIRExprFactor {
                     Operator op = expr->op;
                     switch(op)
                     {
                         // if op is associative and commutative
                         // flatten expression and fold it
                     case Operator::ADD:
                     case Operator::MUL:
                     case Operator::AND:
                     case Operator::OR:
                     case Operator::XOR:
                     case Operator::BAND:
                     case Operator::BOR:
                         {
                             std::vector<FlattenedExpr> operands{};
                             collect_operands(expr->lhs, operands, op);
                             collect_operands(expr->rhs, operands, op);

                             for(auto& flatexpr : operands)
                             {
                                 if(std::holds_alternative<expression_ptr>(flatexpr))
                                 {
                                     expression_ptr_var ptr =
                                         std::move(std::get<expression_ptr>(flatexpr));
                                     HIRExprFactor res = unfold_expression(ptr);
                                     std::visit(Overload{[&flatexpr](VirtualRegisterID& vreg) {
                                                             flatexpr = vreg;
                                                         },
                                                         [&flatexpr](int i) { flatexpr = i; },
                                                         [&flatexpr](std::string_view& view) {
                                                             flatexpr = view;
                                                         },
                                                         [](auto&&) {}},
                                                res);
                                 }
                             }

                             fold_flat_constants(operands, op);
                             return handle_flattened_expr(operands, op);
                         }

                         // try folding constants if both sides of expr are known
                     default:
                         // unfold both subexpressions, but only if necessary.
                         break;
                     }

                     return 1;
                 },
                 [](auto&&) -> HIRExprFactor { return VirtualRegisterID{-1}; }},
        expr);
}
// Flattenedexpr can contain identifiers, virtual registers or integer literals
// there SHOULD not be any expression_ptr's
HIRExprFactor HIRGen::handle_flattened_expr(std::vector<FlattenedExpr>& exprs, Operator op)
{
    if(exprs.size() == 1)
    {
        return std::visit(
            Overload{[](VirtualRegisterID& vreg) -> HIRExprFactor { return vreg; },
                     [this](int& i) -> HIRExprFactor {
                         VirtualRegisterID vreg{current_register_++};
                         hir_stmt_.emplace_back(std::in_place_type<HIRAssign>, vreg, i);
                         return vreg;
                     },
                     [this](std::string_view& view) -> HIRExprFactor {
                         VirtualRegisterID vreg{current_register_++};
                         hir_stmt_.emplace_back(std::in_place_type<HIRLoad>, vreg, view);
                         return vreg;
                     },
                     [](auto&&) -> HIRExprFactor { return VirtualRegisterID{-1}; }},
            exprs[0]);
    }

    HIRExprFactor accumulator = VirtualRegisterID{-1};
    for(auto& flatexpr : exprs)
    {
        std::visit(
            Overload{[this, &accumulator, &op](VirtualRegisterID& vreg) -> void {
                         if(std::holds_alternative<VirtualRegisterID>(accumulator))
                         {
                             VirtualRegisterID acc = std::get<VirtualRegisterID>(accumulator);
                             if(acc.value == -1)
                             {
                                 accumulator = vreg;
                                 return;
                             }
                             VirtualRegisterID res_vreg{current_register_++};
                             hir_stmt_.emplace_back(
                                 std::in_place_type<HIRBinaryOp>, res_vreg, accumulator, op, vreg);
                             accumulator = res_vreg;
                         }
                     },
                     [this, &accumulator, &op](int& i) -> void {
                         if(std::holds_alternative<VirtualRegisterID>(accumulator))
                         {
                             VirtualRegisterID acc = std::get<VirtualRegisterID>(accumulator);
                             if(acc.value == -1)
                             {
                                 accumulator = i;
                                 return;
                             }
                             VirtualRegisterID vreg{current_register_++};
                             hir_stmt_.emplace_back(
                                 std::in_place_type<HIRBinaryOp>, vreg, accumulator, op, i);
                             accumulator = vreg;
                         }
                         accumulator = i;
                     },
                     //  [&accumulator](expression_ptr& expr) -> void {
                     //      accumulator = VirtualRegisterID{-1};
                     //  },
                     [this, &accumulator, &op](std::string_view& view) -> void {
                         if(std::holds_alternative<VirtualRegisterID>(accumulator))
                         {
                             VirtualRegisterID vreg{current_register_++};
                             hir_stmt_.emplace_back(std::in_place_type<HIRLoad>, vreg, view);

                             VirtualRegisterID acc = std::get<VirtualRegisterID>(accumulator);
                             if(acc.value == -1)
                             {
                                 accumulator = vreg;
                                 return;
                             }
                             VirtualRegisterID res_vreg{current_register_++};
                             hir_stmt_.emplace_back(
                                 std::in_place_type<HIRBinaryOp>, res_vreg, accumulator, op, vreg);
                             accumulator = res_vreg;
                         }
                         VirtualRegisterID vreg{current_register_++};
                         hir_stmt_.emplace_back(std::in_place_type<HIRLoad>, vreg, view);
                         accumulator = vreg;
                     },
                     [&accumulator](auto&&) -> void { accumulator = VirtualRegisterID{-1}; }},
            flatexpr);
    }
    return accumulator;
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
    std::visit(Overload{[this](scope_ptr& scope) {
                            std::visit(Overload{[this](std::vector<statements_ptr_var>& vec) {},
                                                [this](auto&&) {}},
                                       scope->stmts);
                        },
                        [this](auto&&) {}},
               node);
}
