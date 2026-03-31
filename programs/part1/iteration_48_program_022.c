// Pseudo-assembly showing the optimization
LOOP_START:
    load trigger into register
    compare trigger > i
    branch_if_true DELAY_SLOT  // Branch takes effect AFTER next instruction
    mov a, 1                   // This executes in the delay slot
    // ... rest of code
