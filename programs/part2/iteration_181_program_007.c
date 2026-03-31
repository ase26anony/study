#include <stdio.h>

/* 
 * This program is designed to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is
 * a simple arithmetic operation that can be safely moved into the delay slot.
 */

int main(int argc, char **argv) {
    /* 
     * Use volatile to prevent constant folding and ensure variables
     * are actually used in computations
     */
    volatile int a = argc + 1;
    volatile int b = argc + 2;
    volatile int c = argc + 3;
    volatile int d = argc + 4;
    volatile int e = argc + 5;
    volatile int f = argc + 6;
    
    /* Additional variables to ensure distinct register sets */
    int x = 0, y = 0, z = 0;
    int r1 = 0, r2 = 0, r3 = 0;
    
    /* 
     * Loop provides scheduling context and prevents elimination
     * Use argc-dependent loop count to prevent optimization
     */
    int iterations = (argc > 1) ? 100 : 200;
    
    for (int i = 0; i < iterations; ++i) {
        /* 
         * Create a runtime-dependent condition to prevent dead code elimination
         * Using modulo with a prime number to create irregular pattern
         */
        if ((i + argc) % 13 == 0) {
            /* 
             * This is the critical jump - it should be a simple conditional jump
             * The condition uses different variables than the target instruction
             */
            goto target_label;
        }
        
        /* Some other computation to prevent optimization */
        x = a + b;
        y = c - d;
        z = e & f;
        
        continue;
        
        /* 
         * Target label with simple, safe instruction
         * This instruction uses different variables than the jump condition
         * to avoid resource conflicts
         */
    target_label:
        /* Simple arithmetic - safe, non-trapping, no special registers */
        r1 = r2 + r3;
        
        /* Additional computation to ensure block isn't trivial */
        r2 = r3 ^ i;
        r3 = r1 | a;
    }
    
    /* 
     * Use results to create observable side effects
     * This prevents dead code elimination of the entire computation
     */
    int checksum = a + b + c + d + e + f + x + y + z + r1 + r2 + r3;
    printf("Result: %d\n", checksum);
    
    return checksum % 256;
}
