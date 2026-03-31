for (insn = BB_HEAD (then_bb);          // Start at first instruction of then block
     insn != NEXT_INSN (then_last_head); // Stop before then_last_head's next instruction
     insn = NEXT_INSN (insn))            // Move to next instruction
