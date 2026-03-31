    jump L1      # insn (has delay slot)
    nop          # delay slot (currently empty)
    ...
L1:
    add r1, r2   # next_trial (candidate for delay slot)
    sub r3, r4
