#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is crafted to trigger specific delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* 
     * Use volatile to prevent constant folding and ensure runtime values
     * are used for condition checks
     */
    volatile int init_val = argc;
    
    /*
     * Declare multiple sets of variables to ensure resource independence:
     * - Variables for the jump condition (x, y, z)
     * - Variables for the target instruction (a, b, c)
     * - Additional variables to prevent optimization (d, e, f)
     */
    int x = 0, y = 0, z = 0;
    int a = 0, b = 0, c = 0;
    int d = 0, e = 0, f = 0;
    
    /* Initialize with runtime-dependent values */
    x = init_val * 3 + 1;
    y = init_val * 5 + 2;
    z = init_val * 7 + 3;
    
    a = init_val * 11 + 4;
    b = init_val * 13 + 5;
    c = init_val * 17 + 6;
    
    d = init_val * 19 + 7;
    e = init_val * 23 + 8;
    f = init_val * 29 + 9;
    
    /* 
     * Loop provides scheduling context and prevents elimination
     * The loop count depends on argc to prevent compile-time optimization
     */
    int loop_count = (argc > 1) ? 100 : 200;
    
    for (int i = 0; i < loop_count; ++i) {
        /* 
         * Complex condition based on runtime values to prevent optimization
         * This creates a conditional jump that should survive to the reorg pass
         */
        if (((i + x) * y - z) % 13 == 0) {
            /* 
             * This goto creates a simplejump_p instruction
             * The target is a label placed before a safe arithmetic operation
             */
            goto target_label;
        }
        
        /* 
         * Alternative path with different computations
         * This ensures the CFG is non-trivial
         */
        d = e ^ f;
        x = y + z;
        
        /* Skip the target instruction in the fall-through path */
        continue;
        
    target_label:
        /*
         * TARGET INSTRUCTION: This is the candidate for delay slot filling
         * - Simple arithmetic operation (addition)
         * - Uses variables not live across the jump (a, b, c are only used here)
         * - Non-trapping operation
         * - Not a jump or complex sequence
         * - Should generate a single RTL pattern
         */
        a = b + c;
        
        /* 
         * Additional operation after the label to ensure the target 
         * instruction isn't the only one in its basic block
         */
        d = e | f;
        
        /* Continue loop execution */
        x = y - z;
    }
    
    /* 
     * Use all modified variables to create observable side effects
     * This prevents dead code elimination
     */
    int checksum = a + b + c + d + e + f + x + y + z;
    
    /* Mix operations to create more instruction variety */
    checksum ^= (a * b) >> 2;
    checksum += (c & d) << 1;
    checksum -= (e | f) ^ 0x55;
    
    /* Final output depends on all computations */
    printf("Result: %d (argc=%d)\n", checksum, argc);
    
    return checksum % 256;
}
