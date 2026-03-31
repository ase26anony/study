#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of variables */
    volatile int cond_base = argc;
    
    /* Variables for the jump condition - separate set */
    int i, mod_val;
    
    /* Variables for the target instruction - completely separate set */
    int a = 0, b = 0, c = 0;
    int d = 0, e = 0, f = 0;
    int x = 0, y = 0, z = 0;
    
    /* Initialize with argc-dependent values to prevent compile-time optimization */
    b = (argc > 0) ? 1 : 0;
    c = (argc > 1) ? 2 : 0;
    e = (argc > 2) ? 3 : 0;
    f = (argc > 3) ? 4 : 0;
    y = (argc > 4) ? 5 : 0;
    z = (argc > 5) ? 6 : 0;
    
    /* Loop to provide scheduling context and prevent elimination */
    int loop_limit = (argc > 1) ? 100 : 200;
    
    for (i = 0; i < loop_limit; ++i) {
        /* Create runtime-dependent condition to prevent dead code elimination */
        mod_val = (i + cond_base) % 13;
        
        /* 
         * Key construct: Conditional jump to label
         * This should generate a simplejump_p instruction
         */
        if (mod_val == 0) {
            goto target_label;
        }
        
        /* Some computation to prevent the block from being trivial */
        x = y ^ z;
        
        /* Continue with other operations */
        d = e & f;
        
        /* Skip the target instruction when not jumping */
        continue;
        
    target_label:
        /* 
         * Target instruction for delay slot filling
         * Simple arithmetic operation with separate variables
         * Must not reference or modify resources tracked in &set and &needed
         */
        a = b + c;
        
        /* Additional operation to ensure target isn't the only instruction */
        d = e ^ f;
    }
    
    /* Use the results to create observable side effects */
    int checksum = a + d + x;
    
    /* Print to prevent dead code elimination */
    printf("Result checksum: %d\n", checksum);
    
    return checksum % 256;
}
