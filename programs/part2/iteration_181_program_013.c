#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is crafted to trigger GCC's delay slot filling logic
 * in the reorg pass, specifically targeting lines 2135-2149 of reorg.cc
 */
int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of condition checks */
    volatile int cond_seed = argc;
    
    /* Multiple sets of variables to ensure resource independence */
    int a = 0, b = 0, c = 0;
    int d = 0, e = 0, f = 0;
    int x = 0, y = 0, z = 0;
    int r1 = 0, r2 = 0, r3 = 0;
    
    /* Initialize with argc-dependent values to prevent compile-time optimization */
    a = (argc > 0) ? 1 : 0;
    b = (argc > 1) ? 2 : 1;
    c = (argc > 2) ? 3 : 2;
    d = (argc > 3) ? 4 : 3;
    e = (argc > 4) ? 5 : 4;
    f = (argc > 5) ? 6 : 5;
    x = argc;
    y = argc + 1;
    z = argc + 2;
    
    /* Loop to provide scheduling context and prevent elimination */
    int iterations = (argc > 1) ? 100 : 200;
    for (int i = 0; i < iterations; ++i) {
        /* Mix computations to create register pressure */
        r1 = a + b;
        r2 = c ^ d;
        r3 = e & f;
        
        /* 
         * KEY CONSTRUCT: Conditional jump that should be a simplejump_p
         * The condition uses runtime values to prevent optimization
         */
        if ((i + cond_seed) % 13 == 0) {
            /* 
             * This goto should create a simple conditional jump
             * The target label is placed immediately before a safe instruction
             */
            goto delay_candidate_target;
        }
        
        /* Alternative path computations */
        x = y + z;
        y = z ^ x;
        z = x & y;
        
        /* Skip the target instruction in the fall-through path */
        goto skip_target;
        
delay_candidate_target:
        /*
         * TARGET INSTRUCTION: This should be the 'next_trial' candidate
         * - Simple arithmetic operation (not a jump)
         * - Uses different variables than the jump condition
         * - Should not reference or set critical resources
         * - Should not trap (safe operations only)
         * - Should be a single RTL pattern (not SEQUENCE)
         */
        a = b + c;  /* Simple register-to-register operation */
        
        /* Additional operations to ensure this isn't a single-instruction block */
        d = e ^ f;
        e = f & a;
        
skip_target:
        /* More computations to create live ranges and prevent optimization */
        r1 = r2 + r3;
        r2 = r3 ^ r1;
        r3 = r1 & r2;
        
        /* Prevent loop unrolling for small iteration counts */
        if (i % 7 == 0) {
            x = x + 1;
        }
    }
    
    /* 
     * Use all variables to create observable side effects
     * This prevents dead code elimination
     */
    int checksum = a + b + c + d + e + f + x + y + z + r1 + r2 + r3;
    
    /* Print result to prevent entire program elimination */
    printf("Result: %d (argc=%d)\n", checksum, argc);
    
    return checksum % 256;
}
