#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of variables */
    volatile int init_val = argc;
    
    /* Separate sets of variables for jump condition and target computation */
    int a = 0, b = 0, c = 0;      /* Used at target label */
    int d = 0, e = 0, f = 0;      /* Used after target label */
    int x = 0, y = 0, z = 0;      /* Used for jump condition */
    
    /* Initialize with argc-dependent values to prevent compile-time optimization */
    a = (init_val * 3) & 0xFF;
    b = (init_val * 5) & 0xFF;
    c = (init_val * 7) & 0xFF;
    d = (init_val * 11) & 0xFF;
    e = (init_val * 13) & 0xFF;
    f = (init_val * 17) & 0xFF;
    x = (init_val * 19) & 0xFF;
    y = (init_val * 23) & 0xFF;
    z = (init_val * 29) & 0xFF;
    
    /* Loop to provide scheduling context and prevent elimination */
    int iterations = (argc > 1) ? 100 : 200;
    int checksum = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Update variables to create data dependencies */
        x = (x + i) & 0xFF;
        y = (y ^ i) & 0xFF;
        z = (z * 3 + i) & 0xFF;
        
        /* 
         * KEY CONSTRUCT: Conditional jump that should be a simplejump_p
         * The condition uses runtime values to prevent optimization
         */
        if (((x + y + z + i + argc) & 0x7) == 0) {
            /* 
             * This goto creates a simple conditional jump to a label.
             * The compiler should recognize this as jump_to_label_p and simplejump_p.
             */
            goto target_label;
        }
        
        /* Some computation to prevent the if block from being eliminated */
        a = (a + 1) & 0xFF;
        b = (b ^ 2) & 0xFF;
        continue;
        
        /* 
         * TARGET LABEL: The instruction here is the candidate for delay slot filling.
         * It must be a simple, non-jump instruction that doesn't reference
         * critical resources and isn't potentially trapping.
         */
    target_label:
        /* 
         * Safe instruction: register-to-register operation
         * Uses different variables than the jump condition
         * Simple addition - no trapping, no condition codes
         */
        a = b + c;  /* Candidate for next_trial */
        
        /* Additional computation to ensure target isn't isolated */
        d = e ^ f;  /* Bitwise operation - also safe */
        
        /* More computations to create a non-trivial basic block */
        checksum = (checksum + a + d) & 0xFFFF;
    }
    
    /* Use all variables to prevent dead code elimination */
    int result = a + b + c + d + e + f + x + y + z + checksum;
    
    /* Print result to create observable side effect */
    printf("Result: %d (argc=%d)\n", result, argc);
    
    return result & 0x7F;  /* Return non-zero to be useful */
}
