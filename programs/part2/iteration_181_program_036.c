#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger GCC's delay slot filling logic
 * in the reorg pass for architectures with delay slots (like MIPS).
 * The key pattern is a conditional jump to a label where the target
 * instruction is safe to move into the delay slot.
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
    for (int i = 0; i < iterations; ++i) {
        /* 
         * Create a runtime-dependent condition that can't be optimized away.
         * The modulo operation with a non-power-of-two prevents optimization.
         */
        if ((i + cond_seed) % 13 == 0) {
            /* 
             * This goto creates a simple conditional jump (simplejump_p).
             * The target label must be immediately before a safe instruction.
             */
            goto target_label;
        }
        
        /* Some other computations to mix control flow */
        x = y ^ z;
        y = z + i;
        z = x | y;
        
        continue;  /* Explicit continue to structure the loop */
        
    target_label:
        /*
         * CRITICAL: This is the candidate instruction for delay slot filling.
         * It must be:
         * 1. A simple non-jump instruction (eligible_for_delay)
         * 2. Not reference or set critical resources (&set, &needed)
         * 3. Not potentially trapping (safe arithmetic)
         * 4. Not a SEQUENCE pattern
         * 
         * Using distinct variables (a,b,c) that aren't live across the jump.
         */
        a = b + c;  /* Simple register-to-register operation */
        
        /* Additional operations to ensure this isn't the only instruction */
        d = e ^ f;
        e = f + 1;
        f = d & e;
        
        /* More computations to create register pressure */
        p = q * 2;
        q = r - 1;
        r = p + q;
    }
    
    /* 
     * Use all modified variables to create observable side effects
     * and prevent dead code elimination.
     */
    int checksum = a + b + c + d + e + f + x + y + z + p + q + r;
    
    /* Print result to ensure variables are used */
    printf("Result: %d (argc=%d)\n", checksum, argc);
    
    return checksum % 256;
}
