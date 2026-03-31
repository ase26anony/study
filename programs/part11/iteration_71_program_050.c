    jump L1
    nop  ; delay slot (empty)
    ...  ; other code
L1:
    add r1, r2, r3  ; This instruction might be movable into the delay slot
