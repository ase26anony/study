    jump L1      ; insn (has delay slot)
    nop          ; delay slot (currently empty)
    ...          ; other code
L1:
    add r1, r2   ; next_trial (instruction at jump target)
    ...          ; rest of code
