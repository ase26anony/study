#include <stdio.h>

/* 
 * This program is crafted to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of variables */
    volatile int init = argc;
    
    /* Declare two distinct sets of variables to ensure resource independence */
    int set1_a, set1_b, set1_c;  /* Used for jump condition */
    int set2_d, set2_e, set2_f;  /* Used at target label */
    int set3_g, set3_h, set3_i;  /* Used after target label */
    
    /* Initialize with argc-dependent values to prevent compile-time optimization */
    set1_a = init + 1;
    set1_b = init + 2;
    set1_c = init + 3;
    
    set2_d = init + 4;
    set2_e = init + 5;
    set2_f = init + 6;
    
    set3_g = init + 7;
    set3_h = init + 8;
    set3_i = init + 9;
    
    /* Result accumulator to prevent dead code elimination */
    int result = 0;
    
    /*
     * Create a loop to provide scheduling context and prevent elimination.
     * The loop count depends on argc to prevent unrolling.
     */
    int loop_limit = (argc > 1) ? 100 : 200;
    
    for (int i = 0; i < loop_limit; ++i) {
        /* 
         * Create a runtime-dependent condition that can't be optimized away.
         * This will generate a conditional jump (simplejump_p).
         */
        if ((i + argc) % 13 == 0) {
            /* 
             * This goto creates a jump to a label.
             * The compiler should see this as a simplejump_p.
             */
            goto target_label;
        }
        
        /* Some computation to make the loop non-trivial */
        set1_a = set1_b ^ set1_c;
        set1_b = set1_c + i;
        set1_c = set1_a - set1_b;
        
        /* Skip the target label code when not jumping */
        continue;
        
    target_label:
        /*
         * This is the candidate instruction for delay slot filling (next_trial).
         * It must be:
         * 1. A non-jump instruction
         * 2. Not a SEQUENCE pattern
         * 3. Not referencing or setting critical resources (&set, &needed)
         * 4. Not potentially trapping
         * 5. Eligible for delay slot
         * 
         * Using simple arithmetic with distinct variables ensures safety.
         */
        set2_d = set2_e + set2_f;  /* Simple add - safe, non-trapping */
        
        /*
         * Additional instruction after the target to ensure it's not the only
         * instruction in the basic block and to use the result.
         */
        set3_g = set2_d ^ set3_h;
        
        /* Continue with loop computation */
        set3_h = set3_i + i;
        set3_i = set3_g - set3_h;
    }
    
    /* 
     * Use all variables in a checksum to prevent dead code elimination
     * and create observable side effects.
     */
    result = set1_a + set1_b + set1_c +
             set2_d + set2_e + set2_f +
             set3_g + set3_h + set3_i;
    
    /* Print result to ensure code isn't optimized away */
    printf("Result: %d\n", result);
    
    return result != 0;
}
