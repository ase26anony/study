#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is crafted to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a
 * safe, non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent constant folding and dead code elimination */
    volatile int init = argc;
    
    /* Declare distinct sets of variables to avoid resource conflicts */
    int a = 0, b = 0, c = 0;      /* Used at target label */
    int d = 0, e = 0, f = 0;      /* Used after target label */
    int x = 0, y = 0, z = 0;      /* Used for jump condition */
    int result = 0;               /* Final checksum */
    
    /* Initialize with runtime-dependent values */
    a = init + 1;
    b = init + 2;
    c = init + 3;
    d = init + 4;
    e = init + 5;
    f = init + 6;
    x = init + 7;
    y = init + 8;
    z = init + 9;
    
    /* Loop provides scheduling context and prevents elimination */
    int iterations = (argc > 1) ? 100 : 200;
    for (int i = 0; i < iterations; ++i) {
        /* 
         * Create a runtime-dependent condition to prevent optimization.
         * The modulo operation with a non-power-of-two prevents simplification.
         */
        if ((i + init) % 13 == 0) {
            /* 
             * Simple conditional jump to label.
             * This should generate a simplejump_p instruction.
             */
            goto target_label;
        }
        
        /* Some computation to make the loop non-trivial */
        x = y ^ z;
        y = z + i;
        z = x & 0xFF;
        
        continue;
        
        /* 
         * Target label with safe instruction for delay slot filling.
         * The instruction must:
         * 1. Not be a jump
         * 2. Not reference or set critical resources (condition codes, stack pointer)
         * 3. Not be potentially trapping
         * 4. Be a single RTL pattern (not SEQUENCE)
         */
    target_label:
        /* Safe, non-trapping arithmetic operation with distinct variables */
        a = b + c;
        
        /* Additional operation to ensure target isn't the only instruction */
        d = e ^ f;
        
        /* More computations to create live ranges */
        x = a + d;
        y = b ^ e;
        z = c & f;
    }
    
    /* 
     * Use all modified variables to create observable side effects
     * and prevent dead code elimination
     */
    result = a + b + c + d + e + f + x + y + z;
    
    /* Print result to ensure variables are used */
    printf("Result: %d\n", result);
    
    return result == 0 ? 0 : 1;
}
