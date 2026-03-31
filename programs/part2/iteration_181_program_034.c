#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is crafted to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a
 * safe, non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of variables */
    volatile int init_val = argc;
    
    /* Distinct sets of variables to avoid resource conflicts */
    int jump_cond_var1, jump_cond_var2;
    int target_var1, target_var2, target_var3;
    int other_var1, other_var2, other_var3;
    
    /* Initialize with runtime-dependent values */
    jump_cond_var1 = init_val * 3;
    jump_cond_var2 = init_val * 7;
    
    target_var1 = init_val + 11;
    target_var2 = init_val + 17;
    target_var3 = 0;
    
    other_var1 = init_val * 5;
    other_var2 = init_val * 13;
    other_var3 = init_val * 19;
    
    /* Loop to provide scheduling context and prevent elimination */
    int loop_limit = (argc > 1) ? 100 : 200;
    int checksum = 0;
    
    for (int i = 0; i < loop_limit; ++i) {
        /* 
         * Create a conditional jump that depends on runtime values.
         * The condition uses modulo with a prime number to ensure
         * it's not predictable at compile time.
         */
        if ((i + jump_cond_var1) % 13 == 0) {
            /* 
             * This should become a simple conditional jump (simplejump_p).
             * The goto target is immediately before a safe instruction.
             */
            goto delay_slot_candidate;
        }
        
        /* Some other computation to make the loop non-trivial */
        other_var1 = other_var2 ^ other_var3;
        other_var2 = other_var1 + i;
        other_var3 = other_var2 * 3;
        
        continue;  /* Explicit continue to structure the control flow */
        
        /* 
         * TARGET LABEL: The instruction here should be eligible for delay slot filling.
         * It uses a DIFFERENT set of variables than the jump condition to avoid
         * resource conflicts with &set and &needed.
         */
        delay_slot_candidate:
        /* Safe, non-trapping arithmetic operation - register-to-register style */
        target_var3 = target_var1 + target_var2;
        
        /* Additional operation to ensure the target isn't isolated */
        target_var1 = target_var2 & target_var3;
        
        /* More computations to prevent dead code elimination */
        other_var1 = other_var1 + target_var3;
        checksum += (i * target_var3) % 256;
    }
    
    /* 
     * Post-loop computations using all variables to create observable side effects
     * and prevent dead code elimination of the entire construct.
     */
    int final_result = 
        jump_cond_var1 + jump_cond_var2 +
        target_var1 + target_var2 + target_var3 +
        other_var1 + other_var2 + other_var3 +
        checksum;
    
    /* Print result to prevent complete optimization */
    printf("Result: %d (argc=%d)\n", final_result, argc);
    
    return final_result % 256;
}
