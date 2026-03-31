Original:
    jump L1      # has delay slot
    nop          # empty delay slot
    ...
L1:  instruction X
    ...

Optimized:
    jump L2      # jump to a jump
    instruction X  # in delay slot
    ...
L1:  instruction X
    ...
L2:  jump L1     # simple jump
