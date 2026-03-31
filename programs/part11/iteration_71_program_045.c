    jump L1
    nop           # delay slot (empty)
    add r1, r2    # some other instruction
L1:
    sub r3, r4    # instruction at jump target
