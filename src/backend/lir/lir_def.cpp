#include "lir_def.hpp"

std::string to_string(LIRInstruction& instr)
{
    switch(instr.opcode)
    {
    case OPCODE::ADD:
        return "ADD " + to_string(instr.dst) + ", " + to_string(instr.left) + ", " +
               to_string(instr.right);
    case OPCODE::SUB:
        return "SUB " + to_string(instr.dst) + ", " + to_string(instr.left) + ", " +
               to_string(instr.right);
    case OPCODE::MUL:
        return "MUL " + to_string(instr.dst) + ", " + to_string(instr.left) + ", " +
               to_string(instr.right);
    case OPCODE::DIV:
        return "DIV " + to_string(instr.dst) + ", " + to_string(instr.left) + ", " +
               to_string(instr.right);
    case OPCODE::MOD:
        return "MOD " + to_string(instr.dst) + ", " + to_string(instr.left) + ", " +
               to_string(instr.right);
    case OPCODE::NEG:
        return "NEG " + to_string(instr.dst) + ", " + to_string(instr.left);
    case OPCODE::LSH:
        return "LSH " + to_string(instr.dst) + ", " + to_string(instr.left) + ", " +
               to_string(instr.right);
    case OPCODE::RSH:
        return "RSH " + to_string(instr.dst) + ", " + to_string(instr.left) + ", " +
               to_string(instr.right);
    case OPCODE::XOR:
        return "XOR " + to_string(instr.dst) + ", " + to_string(instr.left) + ", " +
               to_string(instr.right);
    case OPCODE::OR:
        return "OR " + to_string(instr.dst) + ", " + to_string(instr.left) + ", " +
               to_string(instr.right);
    case OPCODE::AND:
        return "AND " + to_string(instr.dst) + ", " + to_string(instr.left) + ", " +
               to_string(instr.right);
    case OPCODE::NOT:
        return "NOT " + to_string(instr.dst) + ", " + to_string(instr.left);
    case OPCODE::RET:
        return "RET " + to_string(instr.left);
    case OPCODE::CMP:
        return "CMP " + to_string(instr.left) + ", " +
               to_string(instr.right);
    case OPCODE::JE:
        return "JE " + to_string(instr.dst);
    case OPCODE::JNE:
        return "JNE " + to_string(instr.dst);
    case OPCODE::JG:
        return "JG " + to_string(instr.dst);
    case OPCODE::JGE:
        return "JGE " + to_string(instr.dst);
    case OPCODE::JL:
        return "JL " + to_string(instr.dst);
    case OPCODE::JLE:
        return "JLE " + to_string(instr.dst);
    case OPCODE::JZ:
        return "JZ " + to_string(instr.dst);
    case OPCODE::JNZ:
        return "JNZ " + to_string(instr.dst);
    case OPCODE::JMP:
        return "JMP " + to_string(instr.dst);
    case OPCODE::LABEL:
        return to_string(instr.dst);
    case OPCODE::SETE:
        return "SETE " + to_string(instr.dst) + ", " + to_string(instr.left) + ", " +
               to_string(instr.right);
    case OPCODE::SETNE:
        return "SETNE " + to_string(instr.dst) + ", " + to_string(instr.left) + ", " +
               to_string(instr.right);
    case OPCODE::SETG:
        return "SETG " + to_string(instr.dst) + ", " + to_string(instr.left) + ", " +
               to_string(instr.right);
    case OPCODE::SETGE:
        return "SETGE " + to_string(instr.dst) + ", " + to_string(instr.left) + ", " +
               to_string(instr.right);
    case OPCODE::SETL:
        return "SETL " + to_string(instr.dst) + ", " + to_string(instr.left) + ", " +
               to_string(instr.right);
    case OPCODE::SETLE:
        return "SETLE " + to_string(instr.dst) + ", " + to_string(instr.left) + ", " +
               to_string(instr.right);
    case OPCODE::MOV:
        return "MOV " + to_string(instr.dst) + ", " + to_string(instr.left);
    case OPCODE::LEA:
        return "LEA " + to_string(instr.dst) + ", " + to_string(instr.left) + ", " +
               to_string(instr.right);
    default:
        return "UNKNOWN OPCODE";
    }
}

std::string to_string(Operand& operand)
{
    return std::visit(Overload{[](VirtualRegisterID& vreg) -> std::string {
                                   return "T" + std::to_string(vreg.value);
                               },
                               [](int& i) -> std::string {
                                   return std::to_string(i);
                               },
                               [](Register& reg) -> std::string {
                                   return "R" + reg_to_string(reg);
                               },
                               [](LabelID& label) -> std::string {
                                   return "L" + std::to_string(label.value);
                               },
                               [](std::monostate&) -> std::string {
                                   return "empty";
                               },
                               [](StackSlot& slot) -> std::string {
                                   return "[RBP - " + std::to_string(-slot.offset) + "]";
                               },
                               [](auto&&) -> std::string {
                                   return "UNKNOWN OPERAND";
                               }},
                      operand);
}

const std::string& reg_to_string(Register& reg)
{
    return register_map.at(reg);
}
