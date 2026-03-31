for (insn = BB_HEAD (then_bb);           // Start at first instruction of "then" basic block
     insn != NEXT_INSN (then_last_head); // Loop until just past the last head instruction
     insn = NEXT_INSN (insn))            // Move to next instruction
