#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is crafted to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent constant folding and dead code elimination */
    volatile int init = argc;
    
    /* Declare distinct sets of variables to avoid resource conflicts */
    int a = 0, b = 0, c = 0;      /* Used in jump condition */
    int d = 0, e = 0, f = 0;      /* Used at target label */
    int g = 0, h = 0, i = 0;      /* Used after label */
    int result = 0;               /* Final checksum */
    
    /* Initialize with runtime-dependent values */
    a = init + 1;
    b = init * 2;
    c = init | 0x7F;
    d = (init << 3) ^ 0xAA;
    e = (init >> 1) + 5;
    f = init & 0x3F;
    g = init + 7;
    h = init * 3;
    i = init ^ 0xFF;
    
    /* Loop provides scheduling context and prevents elimination */
    int loop_count = (argc > 1) ? 100 : 200;
    for (int iter = 0; iter < loop_count; ++iter) {
        /* 
         * Create a conditional jump that depends on runtime values.
         * The condition uses modulo to create non-trivial control flow.
         */
        if (((iter + argc) % 13) == 0) {
            /* 
             * This goto creates a simplejump_p instruction.
             * The target label must be immediately before a safe instruction.
             */
            goto target_label;
        }
        
        /* Some computation to make the block non-empty */
        a = b + c;
        b = c - a;
        c = a ^ b;
        
        continue;  /* Explicit continue to structure the CFG */
        
    target_label:
        /*
         * CRITICAL: This is the candidate instruction for delay slot filling.
         * It must be:
         * 1. A simple arithmetic/logical operation (not a jump)
         * 2. Not reference or modify critical resources (&set, &needed)
         * 3. Not potentially trapping
         * 4. Use different variables than the jump condition
         */
        d = e + f;  /* Safe: addition of two integers */
        
        /* Additional operations to ensure this isn't a single-instruction block */
        g = h & i;
        h = g | 0x55;
        i = h ^ g;
        
        /* Update result to create observable side effect */
        result += d + g + h + i;
    }
    
    /* 
     * Post-loop computations using modified variables.
     * This ensures variables are live and prevents dead code elimination.
     */
    int checksum = a + b + c + d + e + f + g + h + i + result;
    
    /* Mix with argc to prevent optimization */
    checksum ^= argc;
    
    /* Print result to create observable output */
    printf("Result: %d\n", checksum);
    
    /* Additional volatile operations to affect scheduling */
    volatile int sink = checksum;
    (void)sink;
    
    return (checksum & 0xFF);
}
