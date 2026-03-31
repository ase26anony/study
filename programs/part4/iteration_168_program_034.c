    jump L1
    nop                    # Delay slot (empty)
    ... other code ...
L1:
    add r1, r2, r3        # This instruction could be moved into the delay slot
    sub r4, r5, r6
