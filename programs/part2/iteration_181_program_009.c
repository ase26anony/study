#include <stdio.h>

/* 
 * This program is designed to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of variables */
    volatile int init_val = argc;
    
    /* Declare two distinct sets of variables to ensure resource independence */
    /* Set 1: Used for jump condition - won't be referenced by target instruction */
    int jump_var1 = init_val + 1;
    int jump_var2 = init_val + 2;
    int jump_cond;
    
    /* Set 2: Used only at target label - not live across the jump */
    int target_var1 = init_val * 3;
    int target_var2 = init_val * 5;
    int target_result;
    
    /* Set 3: Used after the label to keep the target block non-trivial */
    int post_var1 = init_val * 7;
    int post_var2 = init_val * 11;
    int post_result;
    
    /* Additional variables to prevent dead code elimination */
    int checksum = 0;
    
    /*
     * Use a loop to provide scheduling context and prevent elimination.
     * The loop count depends on argc to prevent compile-time optimization.
     */
    int loop_limit = (argc > 1) ? 100 : 200;
    
    for (int i = 0; i < loop_limit; ++i) {
        /* 
         * Create a conditional jump based on runtime values.
         * The condition uses modulo with a prime number to ensure
         * it's not predictable at compile time.
         */
        jump_cond = (jump_var1 + i) % 13;
        
        /* 
         * This is the key construct: a conditional jump to a label.
         * The jump must be a simplejump_p (not a conditional jump with delay slot).
         * We use goto to create an unconditional jump after condition check.
         */
        if (jump_cond == 0) {
            /* 
             * Force a simple jump by using goto.
             * This should generate a simplejump_p instruction.
             */
            goto target_label;
        }
        
        /* 
         * Some computation to ensure variables are used
         * and to create register pressure
         */
        jump_var1 = (jump_var1 * 3 + 1) & 0xFF;
        jump_var2 = (jump_var2 * 5 + 1) & 0xFF;
        
        /* Skip the target block when not jumping */
        continue;
        
    target_label:
        /*
         * This is the candidate instruction for delay slot filling.
         * It must be:
         * 1. A non-jump instruction
         * 2. Not a SEQUENCE pattern
         * 3. Not reference or modify resources in &set or &needed
         * 4. Not potentially trapping
         * 5. Eligible for delay slot
         * 
         * We use a simple arithmetic operation with distinct variables
         * that are not used in the jump condition.
         */
        target_result = target_var1 + target_var2;  /* Safe, non-trapping operation */
        
        /*
         * Additional instruction after the target to ensure the block
         * doesn't end immediately and to use the result
         */
        post_result = post_var1 ^ post_var2;  /* Bitwise operation - safe */
        
        /* 
         * Update variables to prevent them from being optimized away
         * and to create data dependencies
         */
        target_var1 = (target_result + 1) & 0xFF;
        target_var2 = (post_result + 1) & 0xFF;
        post_var1 = (post_var1 * 3) & 0xFF;
        post_var2 = (post_var2 * 5) & 0xFF;
        
        /* Update checksum to create observable side effect */
        checksum += target_result + post_result + i;
    }
    
    /* 
     * Additional computations to ensure all variables are used
     * and to prevent dead code elimination
     */
    checksum += jump_var1 + jump_var2 + target_var1 + target_var2 + post_var1 + post_var2;
    
    /* Print result to create observable output */
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
