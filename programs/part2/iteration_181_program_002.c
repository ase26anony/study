#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of condition checks */
    volatile int cond_seed = argc;
    
    /* Use distinct sets of variables to avoid resource conflicts */
    int var_set1_a = 1, var_set1_b = 2, var_set1_c = 3;
    int var_set2_d = 4, var_set2_e = 5, var_set2_f = 6;
    int var_set3_g = 7, var_set3_h = 8, var_set3_i = 9;
    int var_set4_j = 10, var_set4_k = 11, var_set4_l = 12;
    
    /* Result accumulator to prevent dead code elimination */
    int result = 0;
    
    /*
     * Loop provides scheduling context and prevents elimination.
     * The loop count depends on argc to prevent compile-time optimization.
     */
    int loop_count = (argc > 1) ? 100 : 200;
    
    for (int iter = 0; iter < loop_count; ++iter) {
        /* 
         * Create a runtime-dependent condition that can't be optimized away.
         * This ensures the conditional jump remains.
         */
        if ((iter + cond_seed) % 13 == 0) {
            /* 
             * This is the critical conditional jump that should be a simplejump_p.
             * The target label must be immediately before a safe instruction.
             */
            goto delay_slot_candidate_label;
        }
        
        /* Some other computation to mix control flow */
        var_set1_a = var_set1_b + var_set1_c;
        continue;
        
delay_slot_candidate_label:
        /*
         * This is the candidate instruction for delay slot filling (next_trial).
         * It must be:
         * 1. A non-jump instruction
         * 2. Not a SEQUENCE pattern
         * 3. Not referencing or setting critical resources (&set, &needed)
         * 4. Not potentially trapping
         * 5. Eligible for delay slot filling
         * 
         * Using simple arithmetic on distinct variables that aren't live
         * across the jump.
         */
        var_set2_d = var_set2_e + var_set2_f;
        
        /* 
         * Additional computation after the label to ensure the target
         * instruction isn't the only one in its basic block.
         */
        var_set3_g = var_set3_h ^ var_set3_i;
        
        /* Continue with loop computation */
        var_set4_j = var_set4_k | var_set4_l;
    }
    
    /* 
     * Use all modified variables to create observable side effects
     * and prevent dead code elimination.
     */
    result = var_set1_a + var_set2_d + var_set3_g + var_set4_j;
    
    /* Mix with argc to prevent constant folding */
    result ^= argc;
    
    printf("Result: %d\n", result);
    
    /* Additional volatile operations to affect scheduling */
    volatile int dummy = result * 2;
    (void)dummy;
    
    return result & 0xFF;
}
