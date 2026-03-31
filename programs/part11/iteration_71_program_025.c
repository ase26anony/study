    jump L1
    nop                    # ← delay slot (currently empty)
L1:
    add r1, r2, r3        # ← Candidate for moving into delay slot
    ...
