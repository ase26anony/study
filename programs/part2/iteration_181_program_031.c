#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is crafted to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* 
     * Use volatile to prevent optimization of variables used in conditions.
     * This ensures the conditional jump remains intact through optimization passes.
     */
    volatile int cond_base = argc;
    
    /*
     * Use distinct sets of variables to ensure resource independence:
     * - Variables for the jump condition (i, j, k)
     * - Variables for the target instruction computation (a, b, c)
     * - Variables for post-label computation (d, e, f)
     * This prevents resource conflicts in &set and &needed tracking.
     */
    int a = 0, b = 0, c = 0;
    int d = 0, e = 0, f = 0;
    int i, j, k;
    
    /* Initialize with argc-dependent values to prevent compile-time optimization */
    a = argc * 3;
    b = argc * 5;
    c = argc * 7;
    d = argc * 11;
    e = argc * 13;
    f = argc * 17;
    
    /* 
     * Loop provides scheduling context and prevents elimination of the jump.
     * The loop count depends on argc to prevent unrolling or removal.
     */
    int loop_count = (argc > 1) ? 100 : 200;
    
    for (i = 0; i < loop_count; ++i) {
        /* 
         * Create runtime-dependent condition to prevent dead code elimination.
         * The modulo operation with a prime number creates unpredictable branching.
         */
        j = i + cond_base;
        k = j % 13;  /* Prime number to avoid power-of-two optimizations */
        
        /* 
         * KEY CONSTRUCT: Conditional jump that must remain a simplejump_p.
         * The condition uses multiple operations to prevent if-conversion.
         */
        if ((k == 0) || (k == 1) || (k == 2)) {
            /* 
             * Use goto instead of function call to create a direct label reference.
             * This ensures jump_to_label_p(trial) returns true.
             */
            goto target_label;
        }
        
        /* Alternative computation path when jump is not taken */
        a = b - c;
        continue;
        
        /* 
         * TARGET LABEL: Placed immediately before a simple, safe instruction.
         * This instruction (next_trial) must be:
         * 1. A non-jump instruction
         * 2. Not a SEQUENCE pattern
         * 3. Not referencing or setting &set/&needed resources
         * 4. Not potentially trapping
         * 5. Eligible for delay slot filling
         */
    target_label:
        /* Safe, non-trapping arithmetic operation using distinct variables */
        a = b + c;  /* Simple register-to-register operation candidate */
        
        /* Additional operation to ensure target isn't isolated */
        d = e ^ f;  /* Bitwise operation, also safe and non-trapping */
        
        /* Continue loop execution */
    }
    
    /* 
     * Post-loop computation creates observable side effects.
     * This prevents elimination of the entire loop as dead code.
     */
    int checksum = a + b + c + d + e + f + i + j + k;
    
    /* 
     * Use the result to prevent optimization.
     * The printf call ensures variables are live and used.
     */
    printf("Result: %d (argc=%d)\n", checksum, argc);
    
    /* Additional volatile operations to confuse early optimization passes */
    volatile int sink = checksum;
    (void)sink;
    
    return checksum % 256;
}
