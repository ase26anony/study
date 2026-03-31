#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* 
     * Use volatile to prevent constant folding and ensure runtime values
     * This helps preserve the conditional jump structure through optimization passes
     */
    volatile int init_val = argc;
    
    /*
     * Declare two distinct sets of variables to ensure resource independence:
     * - Set 1: Used for the jump condition (i, j, k)
     * - Set 2: Used at the target label (a, b, c, d, e, f)
     * This prevents the candidate instruction from referencing resources in &set/&needed
     */
    int i, j, k;          // For jump condition
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;  // For target computation
    
    /* 
     * Initialize variables with runtime-dependent values
     * This prevents compile-time optimization and dead code elimination
     */
    i = init_val;
    j = argc * 3;
    k = argc + 7;
    
    a = argc * 2;
    b = argc + 1;
    c = argc * 5;
    e = argc * 11;
    f = argc * 13;
    
    /*
     * Use a loop to provide scheduling context and prevent elimination
     * The loop count depends on argc to maintain runtime variability
     */
    int loop_count = (argc > 1) ? 100 : 200;
    
    for (int iter = 0; iter < loop_count; ++iter) {
        /*
         * Create a conditional jump that depends on runtime values
         * The modulo operation with a non-power-of-two prevents optimization
         */
        if (((iter + argc) % 13) == 0) {
            /* 
             * This is the simple conditional jump (simplejump_p)
             * The condition uses variables that are NOT used at the target label
             */
            if ((i + j - k) > 0) {
                /* 
                 * Goto label where candidate instruction resides
                 * This creates the jump_to_label_p pattern
                 */
                goto target_label;
            }
        }
        
        /*
         * Alternate computation path to ensure the target label isn't the only
         * instruction in its basic block
         */
        d = e ^ f;  // Simple, non-trapping operation
        
        /* 
         * Modify condition variables to prevent loop invariant removal
         * These are separate from target label variables
         */
        i = (i * 3 + 1) & 0xFF;
        j = (j + iter) & 0xFF;
        k = (k - argc) & 0xFF;
        
        continue;  // Explicit continue to maintain control flow structure
        
        /*
         * TARGET LABEL: This is where the candidate instruction (next_trial) resides
         * The instruction here must be:
         * 1. A non-jump instruction
         * 2. Not a SEQUENCE pattern
         * 3. Not referencing or setting resources in &set/&needed
         * 4. Not potentially trapping
         * 5. Eligible for delay slot filling
         */
        target_label:
            /* 
             * Simple arithmetic operation using only local variables
             * This is the candidate for delay slot filling (next_trial)
             * Uses variables distinct from the jump condition
             */
            a = b + c;  // Safe, non-trapping, register-to-register operation
            
            /* 
             * Additional instruction after label to ensure it's not just
             * a single instruction block and to use the result
             */
            d = a ^ e;
    }
    
    /*
     * Final computation using all variables to create observable side effects
     * This prevents dead code elimination of the entire construct
     */
    int checksum = a + b + c + d + e + f + i + j + k;
    
    /* 
     * Print result to ensure the code has observable behavior
     * and isn't optimized away entirely
     */
    printf("Result: %d (argc=%d)\n", checksum, argc);
    
    return checksum & 0xFF;
}
