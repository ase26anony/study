#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger GCC's delay slot filling logic
 * in the reorg pass. It creates a specific pattern where:
 * 1. A conditional jump (simplejump_p) targets a label
 * 2. The instruction at the label is a safe, non-jump candidate
 * 3. The candidate doesn't reference critical resources
 * 4. The candidate isn't potentially trapping
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent constant propagation */
    volatile int init_val = argc;
    
    /* Distinct sets of variables to avoid resource conflicts */
    int a = 0, b = 0, c = 0;      /* Used in jump condition */
    int x = 0, y = 0, z = 0;      /* Used at target label */
    int p = 0, q = 0, r = 0;      /* Used after label */
    int result = 0;               /* Final checksum */
    
    /* Initialize with runtime-dependent values */
    a = init_val * 3 + 1;
    b = init_val * 5 + 2;
    c = init_val * 7 + 3;
    
    x = init_val * 11 + 4;
    y = init_val * 13 + 5;
    z = init_val * 17 + 6;
    
    p = init_val * 19 + 7;
    q = init_val * 23 + 8;
    r = init_val * 29 + 9;
    
    /* Loop to provide scheduling context and prevent elimination */
    int iterations = (argc > 1) ? 100 : 200;
    for (int i = 0; i < iterations; ++i) {
        /* 
         * Create a conditional jump that depends on runtime values.
         * The modulo operation prevents compile-time optimization.
         */
        if ((i + a + b) % 13 == 0) {
            /* 
             * This should become a simple conditional jump (simplejump_p).
             * The goto target is immediately before a safe instruction.
             */
            goto target_label;
        }
        
        /* Some computation to prevent the block from being empty */
        p = q ^ r;
        q = r + i;
        r = p * 2;
        
        /* Continue after the target label */
        continue;
        
    target_label:
        /*
         * This is the candidate instruction for delay slot filling.
         * It must be:
         * 1. A non-jump instruction
         * 2. Not a SEQUENCE pattern
         * 3. Not referencing or setting critical resources (&set, &needed)
         * 4. Not potentially trapping
         * 5. Eligible for delay slot
         */
        x = y + z;  /* Simple, safe arithmetic - no trapping possible */
        
        /* Additional computation to ensure this isn't a single-instruction block */
        p = q & r;
        q = r | i;
        r = p << 2;
    }
    
    /* 
     * Use all variables to create observable side effects
     * This prevents dead code elimination
     */
    result = a ^ b ^ c ^ x ^ y ^ z ^ p ^ q ^ r;
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Additional volatile operations to ensure instructions aren't reordered */
    volatile int dummy = result * 2;
    dummy += argc;
    
    return result & 0xFF;
}
