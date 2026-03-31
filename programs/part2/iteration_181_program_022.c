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
    volatile int loop_limit = (argc > 1) ? 100 : 200;
    
    /* Variables for the jump condition - separate set */
    int jump_var1 = argc * 3;
    int jump_var2 = argc + 7;
    int jump_cond = 0;
    
    /* Variables for the target instruction - completely separate set */
    int target_var1 = argc * 5;
    int target_var2 = argc * 11;
    int target_result = 0;
    
    /* Additional variables to prevent elimination */
    int aux_var1 = argc * 13;
    int aux_var2 = argc * 17;
    int aux_result = 0;
    
    /* Checksum to create observable side effects */
    int checksum = 0;
    
    /*
     * Loop provides scheduling context and prevents elimination.
     * The conditional jump inside should create a simplejump_p pattern.
     */
    for (int i = 0; i < loop_limit; ++i) {
        /* Create runtime-dependent condition to prevent optimization */
        jump_cond = (i + cond_base) % 13;
        
        /* 
         * Conditional jump that should be recognized as simplejump_p.
         * The condition uses only the jump-specific variables.
         */
        if (jump_cond == (jump_var1 % 7)) {
            /* 
             * Goto to label where safe instruction resides.
             * This creates the jump_to_label_p pattern.
             */
            goto delay_slot_candidate;
        }
        
        /* Some computation to prevent the block from being empty */
        aux_result = aux_var1 ^ aux_var2;
        checksum += aux_result;
        
        /* Skip past the label to avoid executing it unconditionally */
        goto skip_label;
        
delay_slot_candidate:
        /*
         * Target instruction for delay slot filling.
         * This should be next_trial in the uncovered code.
         * Requirements:
         * 1. Not a jump
         * 2. Simple arithmetic (not trapping)
         * 3. Uses separate variables from jump condition
         * 4. Single RTL pattern (not SEQUENCE)
         */
        target_result = target_var1 + target_var2;  /* Safe addition */
        
        /* Additional operation to ensure block has multiple instructions */
        aux_result = target_result & 0xFF;
        checksum += aux_result;
        
skip_label:
        /* Modify variables to prevent constant propagation */
        target_var1 += i;
        target_var2 ^= i;
        aux_var1 = (aux_var1 * 3) & 0xFFFF;
        aux_var2 = (aux_var2 + 1) & 0xFFFF;
    }
    
    /* 
     * Use results to create observable side effect.
     * This prevents dead code elimination.
     */
    printf("Checksum: %d\n", checksum);
    printf("Target result: %d\n", target_result);
    printf("Aux result: %d\n", aux_result);
    
    return checksum != 0 ? 0 : 1;
}
