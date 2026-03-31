#include <stdio.h>

/* 
 * This program is designed to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* 
     * Use volatile to prevent constant folding and ensure variables
     * survive optimization passes
     */
    volatile int init = argc;
    
    /*
     * Two distinct sets of variables to ensure resource independence:
     * - Set 1: Used for the jump condition (prevents interference with target instruction)
     * - Set 2: Used at the target label (must not reference resources tracked in &set/&needed)
     */
    int cond_a, cond_b;      /* For jump condition */
    int target_x, target_y, target_z;  /* For target instruction */
    int other1, other2, other3;        /* Additional variables to prevent elimination */
    
    /* Initialize with argc-dependent values to prevent compile-time optimization */
    cond_a = init + 1;
    cond_b = init + 2;
    target_x = init + 3;
    target_y = init + 4;
    target_z = init + 5;
    other1 = init + 6;
    other2 = init + 7;
    other3 = init + 8;
    
    /*
     * Create a loop to provide scheduling context and prevent elimination.
     * The loop count depends on argc to prevent unrolling or removal.
     */
    int loop_count = (argc > 1) ? 100 : 200;
    int checksum = 0;
    
    for (int i = 0; i < loop_count; ++i) {
        /*
         * PART 1: Create the conditional jump pattern
         * 
         * The condition must be runtime-dependent to prevent optimization.
         * We use modulo with a prime number to create irregular branching.
         */
        if ((i + argc) % 13 == 0) {
            /* 
             * This goto creates a simplejump_p instruction.
             * The label is placed immediately before a safe arithmetic operation.
             */
            goto delay_slot_candidate;
        }
        
        /*
         * Some intermediate computations to:
         * 1. Use the condition variables (making them live)
         * 2. Prevent the compiler from merging basic blocks
         */
        cond_a = cond_b ^ i;
        cond_b = cond_a + argc;
        
        /* Skip the target instruction when not jumping */
        continue;
        
delay_slot_candidate:
        /*
         * PART 2: The target instruction (next_trial in reorg.cc)
         * 
         * This must be:
         * 1. A non-jump instruction
         * 2. Not a SEQUENCE pattern
         * 3. Not referencing or setting critical resources (&set, &needed)
         * 4. Not potentially trapping
         * 5. Eligible for delay slot filling
         * 
         * Simple register-to-register operation using distinct variables
         * that aren't live across the jump.
         */
        target_z = target_x + target_y;  /* Safe: addition, no trapping */
        
        /*
         * Additional operations after the label to ensure:
         * 1. The target block isn't eliminated
         * 2. The target instruction isn't the only one in its block
         */
        other1 = other2 & other3;        /* Bitwise AND - safe operation */
        other2 = other1 ^ target_z;      /* Mix with target result */
        
        /*
         * Update checksum to create observable side effect
         * and prevent dead code elimination
         */
        checksum += target_z + other1 + other2;
    }
    
    /*
     * PART 3: Post-loop computations
     * 
     * Use all variables to ensure they're live and prevent optimization.
     * This creates register pressure that might affect scheduling.
     */
    int final_result = cond_a + cond_b + target_x + target_y + target_z 
                     + other1 + other2 + other3 + checksum;
    
    /* 
     * Print result to create observable behavior.
     * The actual value isn't important - we just need to use the variables.
     */
    printf("Result: %d\n", final_result % 1000);
    
    return (final_result > 0) ? 0 : 1;
}
