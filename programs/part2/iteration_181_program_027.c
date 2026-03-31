#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is crafted to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of variables used in conditions */
    volatile int cond_base = argc;
    
    /* Distinct sets of variables to avoid resource conflicts */
    int a = 0, b = 0, c = 0;      /* Used at target label */
    int d = 0, e = 0, f = 0;      /* Used after target label */
    int x = 0, y = 0, z = 0;      /* Used in jump condition */
    
    /* Initialize with argc-dependent values to prevent compile-time optimization */
    a = argc * 3;
    b = argc * 5;
    c = argc * 7;
    d = argc * 11;
    e = argc * 13;
    f = argc * 17;
    x = argc * 19;
    y = argc * 23;
    z = argc * 29;
    
    /* Loop provides scheduling context and prevents elimination */
    int iterations = (argc > 1) ? 100 : 200;
    for (int i = 0; i < iterations; ++i) {
        /* 
         * Create a runtime-dependent condition to prevent dead code elimination.
         * The condition uses different variables than those at the target label.
         */
        if (((x + y + z + i) % 13) == (cond_base % 7)) {
            /* 
             * This should generate a simple conditional jump (simplejump_p).
             * The goto target is immediately before a safe arithmetic instruction.
             */
            goto target_label;
        }
        
        /* Some computation to make the loop non-trivial */
        x = (x ^ y) + i;
        y = (y ^ z) * 2;
        z = (z + x) & 0xFF;
        
        continue;  /* Explicit continue to structure the control flow */
        
    target_label:
        /*
         * Target instruction for delay slot filling.
         * - Simple arithmetic (non-trapping)
         * - Uses different variables than the jump condition
         * - Not a jump, not a SEQUENCE
         * - Should not reference/set critical resources
         */
        a = b + c;  /* Simple register-to-register operation candidate */
        
        /* Additional operations to ensure target isn't the only instruction */
        d = e ^ f;
        f = (d + 1) & 0x7F;  /* Safe bitwise operation */
        
        /* Modify condition variables to affect loop behavior */
        x = (x + 1) & 0x3F;
    }
    
    /* 
     * Use all variables to create observable side effects and prevent dead code elimination.
     * The checksum depends on all computations, making them all necessary.
     */
    int checksum = a + b + c + d + e + f + x + y + z;
    
    /* Print result to create observable behavior */
    printf("Result: %d (argc=%d)\n", checksum, argc);
    
    return checksum & 0xFF;
}
