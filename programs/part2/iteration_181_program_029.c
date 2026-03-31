#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of variables */
    volatile int init_val = argc;
    
    /* Separate sets of variables for jump condition and target instruction */
    int cond_var1, cond_var2;      /* Used in jump condition */
    int target_var1, target_var2;  /* Used at target label */
    int other_var1, other_var2;    /* Used elsewhere to prevent elimination */
    int loop_counter, checksum = 0;
    
    /* Initialize with runtime-dependent values */
    cond_var1 = init_val * 3;
    cond_var2 = init_val * 7;
    target_var1 = init_val * 11;
    target_var2 = init_val * 13;
    other_var1 = init_val * 17;
    other_var2 = init_val * 19;
    
    /* Loop provides scheduling context and prevents elimination */
    for (loop_counter = 0; loop_counter < (argc > 1 ? 100 : 200); ++loop_counter) {
        /* 
         * Create a conditional jump that depends on runtime values.
         * The condition uses modulo with a prime number to prevent
         * compile-time optimization.
         */
        if ((loop_counter + cond_var1) % 13 == 0) {
            /* 
             * This goto creates a simplejump_p instruction.
             * The target label must be immediately before a safe instruction.
             */
            goto delay_slot_candidate;
        }
        
        /* Some other computation to make the CFG non-trivial */
        other_var1 = other_var1 ^ other_var2;
        other_var2 = other_var2 + 1;
        
        continue;  /* Explicit continue to maintain structure */
        
        /* 
         * TARGET LABEL: The instruction here is the candidate for delay slot filling.
         * It must be:
         * 1. A non-jump instruction
         * 2. Not a SEQUENCE pattern
         * 3. Not referencing or setting critical resources (&set, &needed)
         * 4. Not potentially trapping
         * 5. Eligible for delay slot
         */
        delay_slot_candidate:
        /* Safe, non-trapping arithmetic operation using different variables */
        target_var1 = target_var1 + target_var2;
        
        /* Additional operation to ensure the block has more than one instruction */
        other_var1 = other_var1 | other_var2;
    }
    
    /* 
     * Use all variables to create observable side effects and prevent dead code elimination.
     * Compute a checksum that depends on all computations.
     */
    checksum = cond_var1 ^ cond_var2 ^ target_var1 ^ target_var2 ^ other_var1 ^ other_var2 ^ loop_counter;
    
    /* Print result to create observable behavior */
    printf("Result: %d (argc=%d)\n", checksum, argc);
    
    return checksum & 0xFF;  /* Return non-zero to prevent optimization */
}
