mem_insn.mem_loc = address_of_x;      // Set memory location to address of x
mem_insn.reg0 = XEXP (x, 0);          // Get first operand of expression x
mem_insn.reg1_is_const = true;        // Flag that second register is constant
mem_insn.reg1_val = 0;                // Set constant value to 0
mem_insn.reg1 = GEN_INT (0);          // Create integer constant 0
if (find_inc (true))                  // Try to find increment with flag=true
    return true;                      // Return success if found
