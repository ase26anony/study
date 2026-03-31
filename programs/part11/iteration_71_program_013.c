    jump L1      # Original jump
    nop          # Delay slot (empty)
    add r1, r2   # Some other instruction
L1:
    add r3, r4   # Instruction at jump target
