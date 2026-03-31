#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to create instruction patterns that trigger
 * GCC's delay slot filling logic in the reorg pass, specifically targeting
 * the uncovered lines in reorg.cc that check for eligible delay slot candidates.
 */

int main(int argc, char **argv) {
    /* 
     * Use volatile to prevent constant folding and ensure variables
     * are actually used in computations
     */
    volatile int init_val = argc;
    
    /*
     * Declare two distinct sets of variables:
     * - Set 1: Used for the jump condition (prevents resource conflicts)
     * - Set 2: Used at the target label (candidate for delay slot)
     */
    int cond_a, cond_b, cond_c;      /* For jump condition */
    int target_x, target_y, target_z; /* For target instruction */
    int other_a, other_b, other_c;   /* Additional variables to prevent elimination */
    
    /* Initialize with argc-dependent values to prevent compile-time optimization */
    cond_a = init_val * 3;
    cond_b = init_val * 5;
    cond_c = init_val * 7;
    
    target_x = init_val * 11;
    target_y = init_val * 13;
    target_z = init_val * 17;
    
    other_a = init_val * 19;
    other_b = init_val * 23;
    other_c = init_val * 29;
    
    /*
     * Use a loop to provide scheduling context and prevent
     * elimination of the jump pattern
     */
    int loop_limit = (argc > 1) ? 100 : 200;
    int checksum = 0;
    
    for (int i = 0; i < loop_limit; ++i) {
        /*
         * Create a runtime-dependent condition that can't be optimized away.
         * The modulo operation with a prime number creates varying results.
         */
        int condition = (i + argc) % 13;
        
        /*
         * Update condition variables to make them live across iterations
         * but not interfere with target variables
         */
        cond_a = (cond_a + i) & 0xFFF;
        cond_b = (cond_b ^ i) & 0xFFF;
        cond_c = cond_c - (i % 5);
        
        /*
         * The key construct: A conditional jump to a label.
         * The condition uses variables that are distinct from those
         * used at the target label.
         */
        if ((condition + cond_a - cond_b + cond_c) > 1000) {
            /* 
             * Use goto to create an explicit jump to label.
             * This should generate a simplejump_p instruction.
             */
            goto target_label;
        }
        
        /* 
         * Some computation after the if to ensure the jump
         * isn't the last instruction in the block
         */
        other_a = other_b + other_c;
        other_b = other_a ^ other_c;
        other_c = other_b - other_a;
        
        /* Skip the target code when not jumping */
        goto skip_target;
        
    target_label:
        /*
         * CRITICAL: This is the candidate instruction for delay slot filling.
         * It must be:
         * 1. A simple, non-jump instruction
         * 2. Not reference resources in &set or &needed
         * 3. Not potentially trapping
         * 4. Eligible for delay slot
         * 
         * Using simple arithmetic with distinct variables that aren't
         * live across the jump.
         */
        target_x = target_y + target_z;  /* Simple add - safe, non-trapping */
        
        /*
         * Additional instructions after the target to ensure it's not
         * isolated and to prevent the block from being optimized away
         */
        target_y = target_x ^ target_z;
        target_z = target_y & 0xFF;
        
    skip_target:
        /* 
         * Use all variables in checksum to prevent dead code elimination
         * and create register pressure
         */
        checksum += cond_a + cond_b + cond_c;
        checksum += target_x + target_y + target_z;
        checksum += other_a + other_b + other_c;
        checksum = checksum & 0xFFFF;  /* Prevent overflow */
        
        /*
         * Occasionally modify target variables to break any potential
         * data dependencies that might prevent scheduling
         */
        if ((i % 7) == 0) {
            target_x = i;
            target_y = i * 2;
            target_z = i * 3;
        }
    }
    
    /*
     * Create observable output to prevent entire program elimination
     * and to verify correct execution
     */
    printf("Checksum: %d\n", checksum);
    printf("Final values - cond: %d,%d,%d target: %d,%d,%d other: %d,%d,%d\n",
           cond_a, cond_b, cond_c,
           target_x, target_y, target_z,
           other_a, other_b, other_c);
    
    return checksum != 0 ? 0 : 1;
}
