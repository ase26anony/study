#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of variables */
    volatile int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    volatile int x = 0, y = 0, z = 0;
    volatile int result = 0;
    
    /* Initialize variables with argc-dependent values to prevent constant folding */
    a = argc + 1;
    b = argc * 2;
    c = argc + 3;
    d = argc * 4;
    e = argc + 5;
    f = argc * 6;
    x = argc + 7;
    y = argc * 8;
    z = argc + 9;
    
    /* Create a loop to provide scheduling context and prevent elimination */
    int iterations = (argc > 1) ? 100 : 200;
    
    for (int i = 0; i < iterations; ++i) {
        /* 
         * Create a conditional jump that depends on runtime values.
         * The condition uses modulo to prevent optimization.
         */
        if ((i + argc) % 13 == 0) {
            /* 
             * This goto creates a simplejump_p instruction.
             * The target label has a safe instruction that should be eligible
             * for delay slot filling.
             */
            goto target_label;
        }
        
        /* Some other computations to create register pressure */
        x = y ^ z;
        y = x + i;
        z = y & 0xFF;
        
        continue;
        
        /* 
         * TARGET LABEL: The instruction here should be eligible for delay slot filling.
         * Requirements:
         * 1. Not a jump
         * 2. Single RTL pattern (not SEQUENCE)
         * 3. Doesn't reference or set critical resources (&set, &needed)
         * 4. Not potentially trapping
         * 5. Eligible for delay slot
         */
    target_label:
        /* Safe, non-trapping arithmetic operation using different variables */
        a = b + c;  /* This is the candidate instruction (next_trial) */
        
        /* Additional operations to ensure the block isn't trivial */
        d = e ^ f;
        e = d + 1;
        
        /* More computations to prevent dead code elimination */
        result += (a + d + e) & 0xFF;
    }
    
    /* Use the results to create observable side effects */
    printf("Result checksum: %d\n", result + a + b + c + d + e + f);
    
    /* Additional branching to increase chances of reorg pass working */
    if (result > 1000) {
        printf("Large result detected\n");
    }
    
    return 0;
}
