#include "lir_generator.hpp"

LIRGenerator::LIRGenerator(std::vector<HIR>& hir_statements,
                           std::map<std::pair<std::string_view, int>, Var, std::less<>>& symbols)
    : hir_statements_(hir_statements), symbols_(symbols)
{
}

LIRGenerator::~LIRGenerator() {}

std::vector<LIRInstruction> LIRGenerator::generate()
{
    for(auto const& [name, var] : symbols_)
    {
        // TODO: better offset alignment
        variable_offsets_[{name.first, name.second}] = current_offset_;
        current_offset_ -= var.type->bytes;
    }

    total_offset_ = ((-current_offset_ + 15) / 16) * 16;

    int index = 0;
    for(auto& hir_stmt : hir_statements_)
    {
        std::visit(
            Overload{
                [&](HIRAssign& assign) -> void {
                    lir_statements_.emplace_back(OPCODE::MOV,
                                                 track_virtual_register(assign.reg),
                                                 lower_factor(assign.lhs),
                                                 std::monostate{});
                },
                [&](HIRUnaryOp& unary_op) -> void {
                    switch(unary_op.op)
                    {
                    case Operator::NOT:
                        {
                            lir_statements_.emplace_back(OPCODE::NOT,
                                                         track_virtual_register(unary_op.reg),
                                                         lower_factor(unary_op.fac),
                                                         std::monostate{});
                            return;
                        }
                    default:
                        return;
                    }
                },
                [&](HIRBinaryOp& binary_op) {
                    cond_ctx_ = {binary_op.reg, binary_op.op, binary_op.rhs, binary_op.lhs, true};
                    switch(binary_op.op)
                    {
                    case Operator::ADD:
                        {
                            lir_statements_.emplace_back(OPCODE::ADD,
                                                         track_virtual_register(binary_op.reg),
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::SUB:
                        {
                            lir_statements_.emplace_back(OPCODE::SUB,
                                                         track_virtual_register(binary_op.reg),
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::MUL:
                        {
                            lir_statements_.emplace_back(OPCODE::MUL,
                                                         track_virtual_register(binary_op.reg),
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::DIV:
                        {
                            lir_statements_.emplace_back(OPCODE::DIV,
                                                         track_virtual_register(binary_op.reg),
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::MOD:
                        {
                            lir_statements_.emplace_back(OPCODE::MOD,
                                                         track_virtual_register(binary_op.reg),
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::LSH:
                        {
                            lir_statements_.emplace_back(OPCODE::LSH,
                                                         track_virtual_register(binary_op.reg),
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::RSH:
                        {
                            lir_statements_.emplace_back(OPCODE::RSH,
                                                         track_virtual_register(binary_op.reg),
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::XOR:
                        {
                            lir_statements_.emplace_back(OPCODE::XOR,
                                                         track_virtual_register(binary_op.reg),
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::OR:
                        {
                            lir_statements_.emplace_back(OPCODE::OR,
                                                         track_virtual_register(binary_op.reg),
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::AND:
                        {
                            lir_statements_.emplace_back(OPCODE::AND,
                                                         track_virtual_register(binary_op.reg),
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::EQ:
                        {
                            lir_statements_.emplace_back(OPCODE::CMP,
                                                         std::monostate{},
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            lir_statements_.emplace_back(OPCODE::SETE,
                                                         track_virtual_register(binary_op.reg),
                                                         std::monostate{},
                                                         std::monostate{});
                            return;
                        }
                    case Operator::NEQ:
                        {
                            lir_statements_.emplace_back(OPCODE::CMP,
                                                         std::monostate{},
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            lir_statements_.emplace_back(OPCODE::SETNE,
                                                         track_virtual_register(binary_op.reg),
                                                         std::monostate{},
                                                         std::monostate{});
                            return;
                        }
                    case Operator::LESS:
                        {
                            lir_statements_.emplace_back(OPCODE::CMP,
                                                         std::monostate{},
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            lir_statements_.emplace_back(OPCODE::SETL,
                                                         track_virtual_register(binary_op.reg),
                                                         std::monostate{},
                                                         std::monostate{});
                            return;
                        }
                    case Operator::LESSEQ:
                        {
                            lir_statements_.emplace_back(OPCODE::CMP,
                                                         std::monostate{},
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            lir_statements_.emplace_back(OPCODE::SETLE,
                                                         track_virtual_register(binary_op.reg),
                                                         std::monostate{},
                                                         std::monostate{});
                            return;
                        }
                    case Operator::GREATER:
                        {
                            lir_statements_.emplace_back(OPCODE::CMP,
                                                         std::monostate{},
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            lir_statements_.emplace_back(OPCODE::SETG,
                                                         track_virtual_register(binary_op.reg),
                                                         std::monostate{},
                                                         std::monostate{});
                            return;
                        }
                    case Operator::GREATEREQ:
                        {
                            lir_statements_.emplace_back(OPCODE::CMP,
                                                         std::monostate{},
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            lir_statements_.emplace_back(OPCODE::SETGE,
                                                         track_virtual_register(binary_op.reg),
                                                         std::monostate{},
                                                         std::monostate{});
                            return;
                        }
                    case Operator::BAND:
                        {
                            lir_statements_.emplace_back(OPCODE::AND,
                                                         track_virtual_register(binary_op.reg),
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::BOR:
                        {
                            lir_statements_.emplace_back(OPCODE::OR,
                                                         track_virtual_register(binary_op.reg),
                                                         lower_factor(binary_op.lhs),
                                                         lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::RETURN:
                        {
                            lir_statements_.emplace_back(OPCODE::RET,
                                                         std::monostate{},
                                                         lower_factor(binary_op.lhs),
                                                         std::monostate{});
                            return;
                        }
                    default:
                        return;
                    }
                },
                [&](HIRJump& jump) {
                    lir_statements_.emplace_back(OPCODE::JMP,
                                                 jump.label,
                                                 std::monostate{},
                                                 std::monostate{});
                },
                [&](HIRCondJump& cond_jump) {
                    if(!cond_ctx_.valid)
                        return;
                    if(index > 0 && std::holds_alternative<HIRBinaryOp>(hir_statements_[index - 1]))
                    {
                        // if previous statement is binary, remove setcc
                        lir_statements_.pop_back();
                    }
                    OPCODE jump_op;
                    switch(cond_ctx_.op)
                    {
                    // OPCODE's are inverted because of the code layout in assembly
                    case Operator::EQ:
                        jump_op = OPCODE::JNE;
                        break;
                    case Operator::NEQ:
                        jump_op = OPCODE::JE;
                        break;
                    case Operator::LESS:
                        jump_op = OPCODE::JG;
                        break;
                    case Operator::LESSEQ:
                        jump_op = OPCODE::JGE;
                        break;
                    case Operator::GREATER:
                        jump_op = OPCODE::JL;
                        break;
                    case Operator::GREATEREQ:
                        jump_op = OPCODE::JLE;
                        break;
                    default:
                        return; // Unsupported operator for conditional jump
                    }

                    // 2. Single emission point
                    lir_statements_.emplace_back(jump_op,
                                                 cond_jump.label,
                                                 std::monostate{},
                                                 std::monostate{});
                },
                [&](LabelID label) {
                    lir_statements_.emplace_back(OPCODE::LABEL,
                                                 label,
                                                 std::monostate{},
                                                 std::monostate{});
                },
                [&](HIRLoad& load) {
                    lir_statements_.emplace_back(
                        OPCODE::MOV,
                        track_virtual_register(load.reg),
                        StackSlot{variable_offsets_[{load.source, load.scope_id}],
                                  symbols_.at({load.source, load.scope_id}).type->bytes},
                        std::monostate{});
                },
                [&](HIRStore& store) {
                    // use rbp offsets
                    lir_statements_.emplace_back(
                        OPCODE::MOV,
                        StackSlot{variable_offsets_[{store.dest, store.scope_id}],
                                  symbols_.at({store.dest, store.scope_id}).type->bytes},
                        lower_factor(store.reg),
                        std::monostate{});
                },
                [&](HIRReturn& ret) {
                    lir_statements_.emplace_back(OPCODE::RET,
                                                 std::monostate{},
                                                 lower_factor(ret.value),
                                                 std::monostate{});
                },
            },
            hir_stmt);
        index++;
    }

    return lir_statements_;
}

Operand LIRGenerator::lower_factor(HIRExprFactor& fac)
{
    return std::visit(Overload{[&](VirtualRegisterID& vreg) -> Operand {
                                   return track_virtual_register(vreg);
                               },
                               [&](int& imm) -> Operand {
                                   return imm;
                               },
                               [&](auto& reg) -> Operand {
                                   return -1;
                               }},
                      fac);
}

int LIRGenerator::final_stack_size()
{ return total_offset_ + max_virtual_register_ * 8; }

int LIRGenerator::variable_stack_size()
{ return total_offset_; }

VirtualRegisterID LIRGenerator::track_virtual_register(VirtualRegisterID& reg)
{
    if(reg.value > max_virtual_register_)
    {
        max_virtual_register_ = reg.value;
    }
    return reg;
}
