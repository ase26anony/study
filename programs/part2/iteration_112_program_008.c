// 1. Remove 'ins' from its current position
SET_NEXT_INSN (prev) = NEXT_INSN (ins);
SET_PREV_INSN (NEXT_INSN (ins)) = prev;

// 2. Insert 'ins' after 'insn'
SET_PREV_INSN (ins) = insn;
SET_NEXT_INSN (ins) = NEXT_INSN (insn);
SET_NEXT_INSN (insn) = ins;

// 3. Update the backward pointer of the node after 'ins'
if (NEXT_INSN (ins))
    SET_PREV_INSN (NEXT_INSN (ins)) = ins;

// 4. Update basic block end if 'insn' was the last instruction
if (BB_END (bb) == insn)
    BB_END (bb) = ins;
