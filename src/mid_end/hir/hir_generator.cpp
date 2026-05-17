#include "hir_generator.hpp"

HIRGen::HIRGen(program_ptr& program,
               std::map<std::pair<std::string_view, int>, Var, std::less<>>& symbols)
    : program_(program), symbols_(symbols), current_register_(0), label_manager_()
{
}

std::vector<HIR>& HIRGen::generate()
{
    for(auto& stmt : program_->stmts) { generate_stmt(stmt); }
    return hir_stmt_;
}

int HIRGen::sethi_ulmann(expression_ptr_var& expr_var)
{
    return std::visit(Overload{[](integer_ptr&) -> int {
                                   return 0;
                               },
                               [](boolean_ptr&) -> int {
                                   return 0;
                               },
                               [](identifier_ptr&) -> int {
                                   return 1;
                               },

                               [this](expression_ptr& expr) -> int {
                                   if(expr->weight != 0)
                                       return expr->weight;

                                   int lhs = sethi_ulmann(expr->lhs);
                                   int rhs = sethi_ulmann(expr->rhs);

                                   if(lhs == rhs)
                                   {
                                       expr->weight = lhs + 1;
                                   }
                                   else
                                   {
                                       expr->weight = std::max(lhs, rhs);
                                   }
                                   return expr->weight;
                               },
                               [](auto&&) -> int {
                                   return 1;
                               }},
                      expr_var);
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
                     hir_stmt_.emplace_back(std::in_place_type<HIRLoad>,
                                            vreg,
                                            ident->name,
                                            ident->scope_id);
                     return vreg;
                 },
                 [this](expression_ptr& expr) -> HIRExprFactor {
                     Operator op = expr->op;
                     //
                     switch(op)
                     {
                     case Operator::ADD:
                     case Operator::MUL:
                     case Operator::AND:
                     case Operator::OR:
                     case Operator::XOR:
                     case Operator::BAND:
                     case Operator::BOR:
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
                             int w_lhs = sethi_ulmann(expr->lhs);
                             int w_rhs = sethi_ulmann(expr->rhs);

                             HIRExprFactor first{0}, second{0};
                             bool flipped = true;

                             if(w_lhs > w_rhs)
                             {
                                 first = unfold_expression(expr->rhs);
                                 second = unfold_expression(expr->lhs);
                             }
                             else
                             {
                                 flipped = false;
                                 first = unfold_expression(expr->lhs);
                                 second = unfold_expression(expr->rhs);
                             }
                             expr->weight = std::max(w_lhs, w_rhs) + 1;

                             VirtualRegisterID vreg{current_register_++};
                             if(flipped)
                             {
                                 hir_stmt_.emplace_back(
                                     std::in_place_type<HIRBinaryOp>, vreg, second, op, first);
                             }
                             else
                             {
                                 hir_stmt_.emplace_back(
                                     std::in_place_type<HIRBinaryOp>, vreg, first, op, second);
                             }
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
void HIRGen::generate_stmt(statements_ptr_var& node)
{
    std::visit(Overload{[this](scope_ptr& scope) {
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
                            bool has_else = _if->else_clause.has_value();
                            LabelID L_NEXT{-1};
                            auto L_END = label_manager_.new_label();
                            if(has_else)
                                L_NEXT = label_manager_.new_label();
                            auto expr = unfold_expression(_if->condition);
                            if(has_else)
                                hir_stmt_.emplace_back(std::in_place_type<HIRCondJump>, expr, L_NEXT);
                            else
                                hir_stmt_.emplace_back(std::in_place_type<HIRCondJump>, expr, L_END);
                            generate_scope_var(_if->scope);
                            if(has_else)
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
                            hir_stmt_.emplace_back(std::in_place_type<HIRStore>,
                                                   expr,
                                                   declassign->ident->name,
                                                   declassign->ident->scope_id);
                        },
                        // TODO: implement declare_ptr; declaration without assignment
                        [this](declare_ptr& declare) {},
                        [this](assign_ptr& assign) {
                            auto expr = unfold_expression(assign->expr);
                            hir_stmt_.emplace_back(std::in_place_type<HIRStore>,
                                                   expr,
                                                   assign->ident->name,
                                                   assign->ident->scope_id);
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
