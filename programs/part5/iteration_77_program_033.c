for (insn = BB_HEAD (then_bb);          // Start at first instruction of basic block
     insn != NEXT_INSN (then_last_head); // Stop before a specific instruction
     insn = NEXT_INSN (insn))           // Iterate through instructions
