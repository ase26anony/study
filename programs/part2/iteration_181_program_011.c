#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is crafted to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a
 * safe candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of condition checks */
    volatile int cond_seed = argc;
    
    /* Distinct sets of variables to avoid resource conflicts */
    int a = 0, b = 0, c = 0;      /* Used in jump condition */
    int x = 0, y = 0, z = 0;      /* Used at target label */
    int p = 0, q = 0, r = 0;      /* Used after target label */
    int result = 0;               /* Final checksum */
    
    /* Initialize with argc-dependent values to prevent compile-time evaluation */
    a = argc * 3;
    b = argc * 5;
    c = argc * 7;
    x = argc * 11;
    y = argc * 13;
    z = argc * 17;
    p = argc * 19;
    q = argc * 23;
    r = argc * 29;
    
    /* Loop provides scheduling context and prevents elimination */
    int iterations = (argc > 1) ? 100 : 200;
    for (int i = 0; i < iterations; ++i) {
        /* 
         * Create a runtime-dependent condition that can't be optimized away.
         * The modulo operation with a non-power-of-two prevents simplification.
         */
        if ((i + cond_seed) % 13 == 0) {
            /* 
             * Simple conditional jump to label.
             * This should generate a simplejump_p instruction.
             */
            goto target_label;
        }
        
        /* Some computations to make the loop non-trivial */
        a = b + c;
        b = c - a;
        c = a ^ b;
        
        continue;  /* Explicit continue to structure the CFG */
        
        /* 
         * TARGET LABEL: The instruction here should be a candidate for delay slot.
         * Requirements:
         * 1. Non-jump instruction
         * 2. Single RTL pattern (not SEQUENCE)
         * 3. Doesn't reference/set critical resources (&set, &needed)
         * 4. Not potentially trapping
         * 5. Eligible for delay slot filling
         */
    target_label:
        /* Safe, non-trapping arithmetic operation with distinct variables */
        x = y + z;  /* Simple register-to-register operation */
        
        /* Additional operation to ensure target isn't isolated */
        p = q ^ r;
        
        /* More computations to prevent dead code elimination */
        y = z * 2;
        z = x >> 1;
        q = r + 1;
        r = p | q;
    }
    
    /* Create observable side-effects to prevent elimination */
    result = a + b + c + x + y + z + p + q + r;
    
    /* Use result to create output */
    printf("Result: %d\n", result);
    
    /* Additional volatile operations to ensure instructions aren't optimized away */
    volatile int sink = result;
    (void)sink;
    
    return result % 256;
}
