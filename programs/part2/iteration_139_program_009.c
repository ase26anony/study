for (insn = BB_HEAD (then_bb);          // Start at first instruction of "then" block
     insn != NEXT_INSN (then_last_head); // Continue until after the last head instruction
     insn = NEXT_INSN (insn))            // Move to next instruction
