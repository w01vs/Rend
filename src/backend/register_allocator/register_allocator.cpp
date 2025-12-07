#include "register_allocator.hpp"

RegisterAllocator::RegisterAllocator(std::vector<LIRInstruction>& lir_statements)
    : lir_statements(lir_statements)
{
}

const std::map<VirtualRegisterID, Register>& RegisterAllocator::allocate()
{
    for(auto& lir_stmt : lir_statements)
    {
        switch(lir_stmt.opcode){
            case OPCODE::SETE:
            case OPCODE::SETNE:
            case OPCODE::SETG:
            case OPCODE::SETGE:
            case OPCODE::SETL:
            case OPCODE::SETLE:
                {
                    break;
                }
        }
    }
}