Original:
    jump L1
    nop           ; delay slot (empty)
    ...           ; other code
L1:
    add r1, r2    ; instruction at jump target

Optimized:
    jump L1
    add r1, r2    ; moved into delay slot
    ...           ; other code
L1:
    nop           ; target now empty (or removed)
