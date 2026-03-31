    jmp   L1      ; Jump instruction with delay slot
    nop           ; Empty delay slot (slots_to_fill = 1, slots_filled = 0)
    ...           ; Other code
    
L1:
    add   r1, r2  ; next_trial - instruction after label
    ...           ; Rest of code at L1
