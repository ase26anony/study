for (insn = BB_HEAD (then_bb);           // Start at first instruction of then_bb
     insn != NEXT_INSN (then_last_head); // Continue until after then_last_head
     insn = NEXT_INSN (insn))            // Move to next instruction
