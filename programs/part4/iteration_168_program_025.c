Original:
    jump L1
    nop           ; delay slot (empty)
    ...           ; other code
    
L1:
    add r1, r2    ; instruction at jump target
    sub r3, r4    ; next instruction

Optimized:
    jump L1
    add r1, r2    ; moved from after L1 to delay slot
    
L1:
    sub r3, r4    ; original next instruction
