    jmp   label   ; branch instruction
    nop           ; delay slot (currently empty)
    ...           ; other code
    
label:
    add   r1, r2  ; instruction at jump target
    ...           ; rest of label code
