    jump L1
    nop              # Empty delay slot
L1:
    add r1, r2, r3   # Could this be moved into the delay slot?
