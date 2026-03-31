Original:
    jump L1
    nop                 # Empty delay slot
L1:
    add r1, r2, r3      # Instruction that could go in delay slot

Optimized:
    jump L1
    add r1, r2, r3      # Filled delay slot
L1:
    # (instruction moved)
