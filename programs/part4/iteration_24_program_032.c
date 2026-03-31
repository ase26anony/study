    jump L1      ; insn with delay slot
    nop          ; empty delay slot (slots_filled = 0)
    ...          ; other code
    
L1:
    add r1, r2   ; next_trial - candidate for delay slot
    ...          ; rest of L1 code
