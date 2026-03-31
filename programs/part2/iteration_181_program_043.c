#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe
 * candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of condition checks */
    volatile int cond_seed = argc;
    
    /* Separate variable sets to avoid resource conflicts */
    int a = 0, b = 0, c = 0;      /* Used in jump condition */
    int d = 0, e = 0, f = 0;      /* Used at target label */
    int g = 0, h = 0, i = 0;      /* Used after label */
    int j = 0, k = 0, l = 0;      /* Additional computation variables */
    
    /* Initialize with argc-dependent values to prevent compile-time optimization */
    a = argc * 3;
    b = argc + 7;
    c = argc - 2;
    d = argc * 5;
    e = argc + 11;
    f = argc * 2;
    g = argc + 3;
    h = argc * 7;
    i = argc - 5;
    j = argc + 13;
    k = argc * 11;
    l = argc + 17;
    
    /* Loop to provide scheduling context and prevent elimination */
    int iterations = (argc > 1) ? 100 : 200;
    for (int loop = 0; loop < iterations; ++loop) {
        /* 
         * Create a runtime-dependent condition that can't be optimized away.
         * This ensures we get a real conditional jump.
         */
        if ((loop + cond_seed) % 13 == 0) {
            /* 
             * This goto creates a simplejump_p instruction.
             * The target label must have a safe, non-jump instruction immediately after.
             */
            goto target_label;
        }
        
        /* Some computation to make the loop non-trivial */
        a = b + c;
        b = c - a;
        c = a ^ b;
        
        /* Skip the target label code when not jumping */
        continue;
        
    target_label:
        /*
         * CRITICAL: This is the candidate instruction (next_trial) for delay slot filling.
         * It must:
         * 1. Be a non-jump instruction
         * 2. Not reference or set resources in &set or &needed
         * 3. Not be potentially trapping
         * 4. Be eligible for delay slot filling
         * 
         * Using simple arithmetic on distinct variables ensures safety.
         */
        d = e + f;  /* Simple add - safe, non-trapping, single RTL pattern */
        
        /* Additional computation to ensure this isn't a basic block with just one instruction */
        g = h & i;   /* Bitwise AND - safe */
        j = k | l;   /* Bitwise OR - safe */
        
        /* More computation to prevent dead code elimination */
        a = d + g;
        b = j - a;
        c = a ^ b;
    }
    
    /* 
     * Use all variables to create observable side effects.
     * This prevents dead code elimination of the entire computation.
     */
    int checksum = a + b + c + d + e + f + g + h + i + j + k + l;
    
    /* Print result to ensure code isn't optimized away */
    printf("Result: %d (argc=%d)\n", checksum, argc);
    
    return checksum % 256;
}
