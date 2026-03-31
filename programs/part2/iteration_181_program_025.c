#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger GCC's delay slot filling logic
 * in the reorg pass. It creates a conditional jump to a label where
 * the instruction at the target is a safe candidate for moving into
 * the delay slot.
 */

int main(int argc, char **argv) {
    /* 
     * Use volatile to prevent constant folding and ensure
     * variables remain live across optimization passes
     */
    volatile int init_val = argc;
    
    /*
     * Create distinct sets of variables:
     * - Variables for the jump condition (jump_*)
     * - Variables for the target instruction computation (target_*)
     * - Additional variables to prevent elimination
     */
    int jump_a, jump_b, jump_c;
    int target_x, target_y, target_z;
    int extra_1, extra_2, extra_3;
    int result = 0;
    
    /* Initialize with argc-dependent values to prevent optimization */
    jump_a = init_val * 3;
    jump_b = init_val + 7;
    jump_c = init_val - 2;
    
    target_x = init_val * 5;
    target_y = init_val + 11;
    target_z = init_val * 2;
    
    extra_1 = init_val + 1;
    extra_2 = init_val * 4;
    extra_3 = init_val - 5;
    
    /*
     * Loop provides scheduling context and prevents elimination
     * of the jump construct
     */
    int loop_limit = (argc > 1) ? 100 : 200;
    for (int i = 0; i < loop_limit; ++i) {
        /*
         * Create a runtime-dependent condition for the jump
         * Using modulo with a prime number to create irregular pattern
         */
        int condition = (i + argc) % 13;
        
        /*
         * First set of operations using jump variables
         * These create values that are used in the condition
         */
        jump_a = jump_b + i;
        jump_b = jump_c ^ i;
        jump_c = jump_a & 0xFF;
        
        /*
         * The key construct: conditional jump to label
         * The condition depends on runtime values to prevent optimization
         */
        if (condition == 0) {
            /* 
             * Use goto instead of if-else to create explicit label
             * This generates a simplejump_p in RTL
             */
            goto target_label;
        }
        
        /*
         * Alternative path when condition is false
         * Uses different variables to maintain separation
         */
        extra_1 = extra_2 | i;
        extra_2 = extra_3 + 1;
        extra_3 = extra_1 ^ 0x55;
        
        /* Skip the target instruction when not jumping */
        goto skip_target;
        
    target_label:
        /*
         * TARGET INSTRUCTION - Candidate for delay slot filling
         * 
         * This is the instruction at the jump target (next_trial).
         * It must be:
         * 1. A non-jump instruction
         * 2. Not a SEQUENCE pattern
         * 3. Not referencing or setting critical resources (&set, &needed)
         * 4. Not potentially trapping
         * 5. Eligible for delay slot
         * 
         * Using simple arithmetic with distinct variables that are
         * not live across the jump
         */
        target_x = target_y + target_z;  /* Simple add - safe, non-trapping */
        
        /*
         * Additional operations after the target to ensure it's not
         * the only instruction in the block
         */
        target_y = target_x & 0x0F;
        target_z = target_y | 0x10;
        
    skip_target:
        /*
         * Continue with other computations to create a non-trivial
         * control flow graph
         */
        result += (jump_a + target_x + extra_1) & 0xFF;
        
        /*
         * Re-initialize target variables to break dependencies
         * and ensure they're not live across loop iterations
         */
        if (i % 5 == 0) {
            target_x = init_val + i;
            target_y = init_val * (i + 1);
            target_z = init_val - i;
        }
    }
    
    /*
     * Use the computed values to create observable side effects
     * This prevents dead code elimination
     */
    int checksum = result + jump_a + target_x + extra_1;
    
    /* 
     * Print checksum to create external side effect
     * Use argc to make output variable
     */
    if (argc > 1) {
        printf("Checksum (with args): %d\n", checksum);
    } else {
        printf("Checksum (no args): %d\n", checksum);
    }
    
    return checksum & 0x7F;  /* Return non-zero to indicate execution */
}
