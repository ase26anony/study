mem_insn.mem_loc = address_of_x;      // Memory location being accessed
mem_insn.reg0 = XEXP (x, 0);          // First operand (likely a register)
mem_insn.reg1_is_const = true;        // Second operand is constant
mem_insn.reg1_val = 0;                // Constant value = 0
mem_insn.reg1 = GEN_INT (0);          // Generate integer constant 0
