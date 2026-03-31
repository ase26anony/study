#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is crafted to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of condition checks */
    volatile int cond_seed = argc;
    
    /* Multiple sets of variables to ensure resource independence */
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    int x = 1, y = 2, z = 3;
    int p = 4, q = 5, r = 6;
    
    /* Initialize with argc-dependent values to prevent constant folding */
    a = (argc > 0) ? 10 : 20;
    b = (argc > 1) ? 15 : 25;
    c = (argc > 2) ? 20 : 30;
    d = (argc > 3) ? 25 : 35;
    e = (argc > 4) ? 30 : 40;
    f = (argc > 5) ? 35 : 45;
    
    /* Loop provides scheduling context and prevents elimination */
    int iterations = (argc > 1) ? 100 : 200;
    int checksum = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* 
         * Create a runtime-dependent condition that can't be optimized away.
         * The modulo operation with a non-power-of-two prevents simplification.
         */
        if ((i + cond_seed) % 13 == 0) {
            /* 
             * This is the critical jump - a simple conditional jump to a label.
             * The compiler should generate a simplejump_p instruction here.
             */
            goto target_label;
        }
        
        /* Some computations to mix up the control flow */
        x = y ^ z;
        y = z + i;
        z = x & 0xFF;
        
        /* Skip the target instruction when not jumping */
        continue;
        
    target_label:
        /*
         * This is the candidate instruction for delay slot filling (next_trial).
         * It must be:
         * 1. A non-jump instruction
         * 2. Not a SEQUENCE pattern
         * 3. Not referencing or setting critical resources (&set, &needed)
         * 4. Not potentially trapping
         * 5. Eligible for delay slot
         * 
         * Using simple arithmetic with distinct variables ensures safety.
         */
        a = b + c;  /* Simple, non-trapping, register-to-register operation */
        
        /* Additional operations to ensure this isn't a single-instruction block */
        d = e ^ f;
        p = q | r;
        
        /* Update checksum to create observable side effect */
        checksum += a + d + p;
    }
    
    /* 
     * Additional computations using modified variables to prevent dead code 
     * elimination and create observable output
     */
    int result = a + b + c + d + e + f + x + y + z + p + q + r + checksum;
    
    /* Print result to prevent optimization and verify execution */
    printf("Result: %d (argc=%d)\n", result, argc);
    
    return (result > 0) ? 0 : 1;
}
