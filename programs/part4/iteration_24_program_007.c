    jmp   L1      ; jump with delay slot
    nop           ; empty delay slot
    ...
L1:
    add   r1, r2  ; instruction at jump target
