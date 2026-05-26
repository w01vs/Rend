#ifndef REGISTER_ALLOCATOR_HPP
#define REGISTER_ALLOCATOR_HPP

#include "../lir/lir_def.hpp"

class RegisterAllocator {
  public:
    RegisterAllocator(std::vector<LIRInstruction>& lir_statements);

    const std::map<VirtualRegisterID, Register>& allocate();


  private:
    std::map<Register, VirtualRegisterID> register_map;
    std::map<VirtualRegisterID, std::variant<Register, StackSlot>> address_map;
    std::vector<LIRInstruction>& lir_statements;

};

#endif // REGISTER_ALLOCATOR_HPP