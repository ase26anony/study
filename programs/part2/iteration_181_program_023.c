#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to create instruction patterns that trigger
 * GCC's delay slot filling logic in the reorg pass, specifically targeting
 * the uncovered lines in reorg.cc (lines 2135-2149).
 */

int main(int argc, char **argv) {
    /* 
     * Use volatile to prevent constant folding and ensure variables
     * are actually used in computations
     */
    volatile int init_val = argc;
    
    /* 
     * Declare multiple sets of variables to ensure resource independence:
     * - Variables for the jump condition (cond_*)
     * - Variables for the target instruction (target_*)
     * - Variables for other computations (other_*)
     */
    int cond_a, cond_b, cond_result;
    int target_x, target_y, target_z;
    int other_p, other_q, other_r;
    int checksum = 0;
    
    /* Initialize with argc-dependent values to prevent optimization */
    cond_a = init_val * 3;
    cond_b = init_val * 7;
    target_x = init_val * 11;
    target_y = init_val * 13;
    other_p = init_val * 17;
    other_q = init_val * 19;
    other_r = init_val * 23;
    
    /*
     * Create a loop to provide scheduling context and prevent
     * elimination of the jump pattern
     */
    int loop_limit = (argc > 1) ? 100 : 200;
    
    for (int i = 0; i < loop_limit; ++i) {
        /* 
         * Create a runtime-dependent condition for the jump.
         * Using modulo with a prime number to create irregular pattern
         */
        cond_result = (i + argc) % 13;
        
        /* 
         * The key construct: a conditional jump to a label.
         * This should generate a simplejump_p instruction.
         */
        if (cond_result == 0) {
            /* 
             * Use goto to create an explicit jump to label.
             * This is more likely to create the simple jump pattern
             * than a structured if statement.
             */
            goto delay_slot_candidate;
        }
        
        /* 
         * Alternative path when jump is not taken.
         * Perform computations to ensure variables are live.
         */
        other_p = other_q ^ other_r;
        other_q = other_r + i;
        other_r = other_p * 2;
        
        /* Skip the target instruction when not jumping */
        goto skip_target;
        
delay_slot_candidate:
        /*
         * Target instruction for delay slot filling.
         * This must be:
         * 1. A non-jump instruction
         * 2. Not referencing or setting critical resources (&set, &needed)
         * 3. Not potentially trapping
         * 4. A simple RTL pattern (not SEQUENCE)
         */
        target_z = target_x + target_y;  /* Simple, safe arithmetic */
        
        /*
         * Additional instruction after target to ensure it's not
         * the only instruction in the basic block
         */
        other_p = other_q | other_r;
        
skip_target:
        /* 
         * Continue with loop body to create register pressure
         * and scheduling opportunities
         */
        cond_a = cond_b - i;
        cond_b = cond_a * 3;
        
        target_x = target_y ^ i;
        target_y = target_x + 1;
        
        /* Update checksum to prevent dead code elimination */
        checksum += cond_result + target_z + other_p;
    }
    
    /*
     * Additional computations to ensure all variables are used
     * and to create observable output
     */
    int final_result = checksum + cond_a + cond_b + target_x + target_y + 
                      target_z + other_p + other_q + other_r;
    
    /* 
     * Print result to create side effect and prevent optimization.
     * Use volatile pointer to ensure actual memory access.
     */
    volatile int *output = (volatile int *)malloc(sizeof(int));
    *output = final_result;
    printf("Result: %d (checksum: %d)\n", *output, checksum);
    
    free((void *)output);
    
    return (final_result > 0) ? 0 : 1;
}
