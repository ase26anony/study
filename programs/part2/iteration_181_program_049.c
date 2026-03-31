#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is crafted to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is
 * a safe, non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* 
     * Use volatile to prevent constant folding and ensure variables
     * survive optimization passes
     */
    volatile int init_val = argc;
    
    /*
     * Two distinct sets of variables:
     * - Set 1: Used for the jump condition (x, y, z)
     * - Set 2: Used at the target label (a, b, c, d, e, f)
     * This ensures resource independence
     */
    int x = 0, y = 0, z = 0;
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    
    /* Initialize with runtime-dependent values */
    x = init_val + 1;
    y = init_val * 2;
    z = init_val | 0x7F;
    
    a = init_val + 3;
    b = init_val * 4;
    c = init_val ^ 0x55;
    d = init_val + 5;
    e = init_val * 6;
    f = init_val & 0xAA;
    
    /*
     * Loop provides scheduling context and prevents elimination
     * The loop count depends on argc to prevent compile-time determination
     */
    int iterations = (argc > 1) ? 100 : 200;
    int checksum = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* 
         * Complex condition prevents if-conversion to conditional moves
         * The modulo operation with a non-power-of-two prevents optimization
         */
        if (((i + argc) % 13) == 0) {
            /* 
             * This should generate a simple conditional jump (simplejump_p)
             * The goto prevents tail merging and preserves the jump structure
             */
            goto target_label;
        }
        
        /* Some computation to make the loop non-trivial */
        x = y + z;
        y = z ^ i;
        z = x & 0xFF;
        
        continue;  /* Explicit continue to separate basic blocks */
        
        /* 
         * TARGET LABEL: The instruction here should be eligible for delay slot
         * Must be a simple, non-jump, non-trapping instruction
         * Using different variables than the jump condition
         */
    target_label:
        /* Simple arithmetic - safe, non-trapping, single RTL pattern */
        a = b + c;
        
        /* Additional operation to ensure target isn't the only instruction */
        d = e ^ f;
        
        /* More computations to create register pressure */
        checksum += (a + d) & 0x3F;
        f = (e << 2) | 0x1;
        e = d - 1;  /* Safe subtraction (won't trap) */
    }
    
    /* 
     * Use all variables to prevent dead code elimination
     * Create observable side effect
     */
    int final_result = (x + y + z + a + b + c + d + e + f + checksum) & 0xFFFF;
    
    printf("Result: %d (argc=%d)\n", final_result, argc);
    
    /* Additional volatile operations to prevent reordering */
    volatile int sink = final_result;
    
    return final_result != 0;
}
