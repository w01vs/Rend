#include "hir_generator.hpp"

HIRGen::HIRGen(program_ptr& program, std::map<std::string_view, Var, std::less<>>& symbols)
    : program_(program), symbols_(symbols), current_register_(0), label_manager_()
{
}

std::vector<HIR>& HIRGen::generate()
{
    for(auto& stmt : program_->stmts) { generate_stmt(stmt); }
    return hir_stmt_;
}

void HIRGen::fold_comm_assoc(std::vector<FlattenedExpr>& operands, Operator op) const
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
            break;
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
            break;
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
            break;
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
            break;
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
            break;
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
            break;
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
            break;
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
                        [&operands](identifier_ptr& ident) {
                            operands.emplace_back(ident->name);
                        },
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

HIRExprFactor HIRGen::fold_other(int lhs, int rhs, Operator op) const
{
    switch(op)
    {
    case Operator::SUB:
        return lhs - rhs;
    case Operator::DIV:
        return lhs / rhs;
    case Operator::MOD:
        return lhs % rhs;
    case Operator::LESS:
        return lhs < rhs ? 1 : 0;
    case Operator::GREATER:
        return lhs > rhs ? 1 : 0;
    case Operator::LESSEQ:
        return lhs <= rhs ? 1 : 0;
    case Operator::GREATEREQ:
        return lhs >= rhs ? 1 : 0;
    case Operator::EQ:
        return lhs == rhs ? 1 : 0;
    case Operator::NEQ:
        return lhs != rhs ? 1 : 0;
    case Operator::LSH:
        return lhs << rhs;
    case Operator::RSH:
        return lhs >> rhs;
    default:
        return VirtualRegisterID{-1};
    }
}

HIRExprFactor HIRGen::unfold_expression(expression_ptr_var& expr)
{
    return std::visit(
        Overload{[](integer_ptr& integer) -> HIRExprFactor {
                     return integer->value;
                 },
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
                     case Operator::ADD:
                     case Operator::MUL:
                     case Operator::AND:
                     case Operator::OR:
                     case Operator::XOR:
                     case Operator::BAND:
                     case Operator::BOR:
                         {
                             // if op is associative and commutative
                             // flatten expression and fold it
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
                                                         [&flatexpr](int i) {
                                                             flatexpr = i;
                                                         },
                                                         [&flatexpr](std::string_view& view) {
                                                             flatexpr = view;
                                                         },
                                                         [](auto&&) {}},
                                                res);
                                 }
                             }

                             fold_comm_assoc(operands, op);
                             return handle_flattened_expr(operands, op);
                         }

                     case Operator::SUB:
                     case Operator::DIV:
                     case Operator::MOD:
                     case Operator::LESS:
                     case Operator::GREATER:
                     case Operator::LESSEQ:
                     case Operator::GREATEREQ:
                     case Operator::EQ:
                     case Operator::NEQ:
                     case Operator::LSH:
                     case Operator::RSH:
                         {
                             // try folding constants if both sides of expr are known
                             std::pair<HIRExprFactor, HIRExprFactor> sides{
                                 unfold_expression(expr->lhs), unfold_expression(expr->rhs)};
                             if(std::holds_alternative<int>(sides.first) &&
                                std::holds_alternative<int>(sides.second))
                             {
                                 return fold_other(std::get<int>(sides.first),
                                                   std::get<int>(sides.second),
                                                   op);
                             }

                             VirtualRegisterID vreg{current_register_++};
                             hir_stmt_.emplace_back(std::in_place_type<HIRBinaryOp>,
                                                    vreg,
                                                    sides.first,
                                                    op,
                                                    sides.second);
                             return vreg;
                         }
                     case Operator::NOT:
                         {
                             HIRExprFactor res = unfold_expression(expr->lhs);
                             if(std::holds_alternative<int>(res))
                             {
                                 int val = std::get<int>(res);
                                 return val == 0 ? 1 : 0;
                             }
                             VirtualRegisterID vreg{current_register_++};
                             hir_stmt_.emplace_back(std::in_place_type<HIRUnaryOp>,
                                                    vreg,
                                                    res,
                                                    Operator::NOT);
                             return vreg;
                         }
                     default:
                         // what is this?!
                         std::cerr << "Unhandled operator in unfold_expression\n";
                         break;
                     }

                     return 1;
                 },
                 [](auto&&) -> HIRExprFactor {
                     return VirtualRegisterID{-1};
                 }},
        expr);
}
// Flattenedexpr can contain identifiers, virtual registers or integer literals
// there SHOULD not be any expression_ptr's
HIRExprFactor HIRGen::handle_flattened_expr(std::vector<FlattenedExpr>& exprs, Operator op)
{
    if(exprs.size() == 1)
    {
        return std::visit(
            Overload{[](VirtualRegisterID& vreg) -> HIRExprFactor {
                         return vreg;
                     },
                     [this](int& i) -> HIRExprFactor {
                         VirtualRegisterID vreg{current_register_++};
                         hir_stmt_.emplace_back(std::in_place_type<HIRAssign>, vreg, i);
                         return i;
                     },
                     [this](std::string_view& view) -> HIRExprFactor {
                         VirtualRegisterID vreg{current_register_++};
                         hir_stmt_.emplace_back(std::in_place_type<HIRLoad>, vreg, view);
                         return vreg;
                     },
                     [](auto&&) -> HIRExprFactor {
                         return VirtualRegisterID{-1};
                     }},
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
                     },
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
                     [&accumulator](auto&&) -> void {
                         accumulator = VirtualRegisterID{-1};
                     }},
            flatexpr);
    }
    return accumulator;
}

void HIRGen::generate_stmt(statements_ptr_var& node)
{
    std::visit(
        Overload{[this](scope_ptr& scope) {
                     generate_inner_scope(scope->stmts);
                 },
                 [this](break_ptr& _break) {
                     LabelID exitLabel = label_manager_.get_loop_exit_label();
                     hir_stmt_.emplace_back(std::in_place_type<HIRJump>, exitLabel);
                 },
                 [this](continue_ptr& _continue) {
                     LabelID testLabel = label_manager_.get_loop_test_label();
                     hir_stmt_.emplace_back(std::in_place_type<HIRJump>, testLabel);
                 },
                 [this](return_ptr& _return) {
                     auto expr = unfold_expression(_return->val);
                     hir_stmt_.emplace_back(std::in_place_type<HIRReturn>, expr);
                 },
                 [this](if_ptr& _if) {
                     auto L_END = label_manager_.new_label();
                     auto L_NEXT = label_manager_.new_label();
                     auto expr = unfold_expression(_if->condition);
                     hir_stmt_.emplace_back(std::in_place_type<HIRCondJump>, expr, L_NEXT);
                     generate_scope_var(_if->scope);
                     if(_if->else_clause.has_value())
                     {
                         hir_stmt_.emplace_back(std::in_place_type<HIRJump>, L_END);
                         hir_stmt_.emplace_back(std::in_place_type<LabelID>, L_NEXT);
                         generate_else_var(_if->else_clause.value(), L_END);
                     }
                     hir_stmt_.emplace_back(std::in_place_type<LabelID>, L_END);
                 },
                 [this](while_ptr& _while) {
                     auto L_TEST = label_manager_.test_loop();
                     auto L_EXIT = label_manager_.start_loop();
                     hir_stmt_.emplace_back(std::in_place_type<LabelID>, L_TEST);
                     auto expr = unfold_expression(_while->condition);
                     hir_stmt_.emplace_back(std::in_place_type<HIRCondJump>, expr, L_EXIT);
                     generate_scope_var(_while->scope);
                     hir_stmt_.emplace_back(std::in_place_type<HIRJump>, L_TEST);
                     hir_stmt_.emplace_back(std::in_place_type<LabelID>, L_EXIT);
                     label_manager_.end_loop();
                 },
                 [](struct_ptr& _struct) {},
                 [this](declareassign_ptr& declassign) {
                     auto expr = unfold_expression(declassign->expr);
                     hir_stmt_.emplace_back(std::in_place_type<HIRStore>, expr, declassign->name);
                 },
                 [this](declare_ptr& declare) {},
                 [this](assign_ptr& assign) {
                     auto expr = unfold_expression(assign->expr);
                     hir_stmt_.emplace_back(std::in_place_type<HIRStore>, expr, assign->name);
                 },
                 [this](auto&&) {}},
        node);
}

void HIRGen::generate_else_var(else_ptr_var& node, LabelID L_END)
{
    std::visit(Overload{[this, L_END](else_ptr& _else) {
                            if(_else->condition.has_value())
                            {
                                auto L_NEXT = label_manager_.new_label();
                                auto expr = unfold_expression(_else->condition.value());
                                hir_stmt_.emplace_back(std::in_place_type<HIRCondJump>,
                                                       expr,
                                                       L_NEXT);
                                generate_scope_var(_else->scope);
                                hir_stmt_.emplace_back(std::in_place_type<HIRJump>, L_END);
                                hir_stmt_.emplace_back(std::in_place_type<LabelID>, L_NEXT);
                                if(_else->else_if_clause.has_value())
                                {
                                    generate_else_var(_else->else_if_clause.value(), L_END);
                                }
                            }
                            else
                            {
                                generate_scope_var(_else->scope);
                            }
                        },
                        [this](auto&&) {}},
               node);
}

void HIRGen::generate_inner_scope(stmt_vec_err_ptr& node)
{
    std::visit(Overload{[this](std::vector<statements_ptr_var>& vec) {
                            for(auto& stmt : vec) { generate_stmt(stmt); }
                        },
                        [this](auto&&) {}},
               node);
}

void HIRGen::generate_scope_var(scope_err_ptr_var& node)
{
    std::visit(Overload{[this](scope_ptr& scope) {
                            std::visit(Overload{[this](std::vector<statements_ptr_var>& vec) {
                                                    for(auto& stmt : vec) { generate_stmt(stmt); }
                                                },
                                                [this](auto&&) {}},
                                       scope->stmts);
                        },
                        [this](auto&&) {}},
               node);
}
