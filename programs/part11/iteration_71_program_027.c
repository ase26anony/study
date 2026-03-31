Original:
    jump L1
    nop            # delay slot (empty)
    ...            # fall-through code
L1:
    add r1, r2, r3 # instruction at jump target
    ...

After optimization:
    jump L1
    add r1, r2, r3 # delay slot filled from jump target
    ...            # fall-through code
L1:
    ...            # (add instruction removed from here)
