#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger GCC's delay slot filling logic
 * in reorg.cc lines 2135-2149. It creates a conditional jump to a label
 * where the instruction at the label is a safe candidate for moving into
 * the delay slot.
 */

int main(int argc, char **argv) {
    /* 
     * Use volatile to prevent constant folding and ensure variables
     * are actually used in computations
     */
    volatile int init_val = argc;
    
    /*
     * Declare two distinct sets of variables to ensure resource independence:
     * - Set 1: Used for the jump condition (i, j, k)
     * - Set 2: Used at the target label (a, b, c, d, e, f)
     * This prevents the candidate instruction from referencing resources
     * tracked in &set and &needed
     */
    int i, j, k;
    int a, b, c, d, e, f;
    
    /* Initialize with runtime-dependent values to prevent optimization */
    i = init_val * 3;
    j = init_val * 5;
    k = init_val * 7;
    
    a = init_val * 11;
    b = init_val * 13;
    c = init_val * 17;
    d = init_val * 19;
    e = init_val * 23;
    f = init_val * 29;
    
    /* 
     * Create a loop to provide scheduling context and prevent 
     * elimination of the jump pattern
     */
    int loop_count = (argc > 1) ? 100 : 200;
    int checksum = 0;
    
    for (int iter = 0; iter < loop_count; ++iter) {
        /* 
         * First, do some computation with the first set of variables
         * to create register pressure and scheduling opportunities
         */
        i = j + k;
        j = k - i;
        k = i ^ j;
        
        /*
         * KEY CONSTRUCT: Conditional jump that should be recognized as
         * a simplejump_p to a label
         * 
         * The condition uses runtime values to prevent dead code elimination
         * The modulo operation with a non-power-of-two prevents optimization
         */
        if (((iter + argc) % 13) == 0) {
            /* 
             * This goto creates a simple conditional jump to target_label
             * The compiler should recognize this as jump_to_label_p(trial)
             * and simplejump_p(trial)
             */
            goto target_label;
        }
        
        /* 
         * Alternate path when condition is false
         * Use different computations to create distinct basic blocks
         */
        a = b - c;
        d = e | f;
        goto continue_loop;
        
    target_label:
        /*
         * TARGET INSTRUCTION CANDIDATE (next_trial):
         * This is the instruction we want to be considered for delay slot filling
         * 
         * Requirements met:
         * 1. NONJUMP_INSN_P(next_trial) - simple arithmetic, not a jump
         * 2. Not a SEQUENCE pattern - simple assignment
         * 3. Does not reference or set critical resources (&set, &needed):
         *    - Uses only general purpose variables a, b, c
         *    - No condition codes, stack pointer, or special registers
         * 4. Not potentially trapping: addition of integers, no division
         * 5. Eligible for delay: simple register-to-register operation
         */
        a = b + c;  /* Simple, safe arithmetic - candidate for delay slot */
        
        /*
         * Additional instruction after the label to ensure the target
         * is not the only instruction in its basic block
         */
        d = e ^ f;
        
    continue_loop:
        /* 
         * Use the results to prevent dead code elimination
         * Mix computations to create complex control flow
         */
        checksum += (a ^ b) + (c & d) - (e | f);
        
        /* 
         * Additional branching to create more scheduling opportunities
         * and prevent if-conversion
         */
        if ((checksum & 1) == 0) {
            e = f * 3;  /* Safe multiplication, no overflow issues */
        } else {
            f = e / 2;  /* Safe division by constant 2 */
        }
        
        /* 
         * Force variable liveness across iterations to maintain
         * register pressure but ensure target instruction variables
         * are not live across the jump
         */
        int temp = a;
        a = b;
        b = c;
        c = temp;
    }
    
    /* 
     * Create observable side effects to prevent entire loop elimination
     * and to verify the program executed correctly
     */
    printf("Final checksum: %d\n", checksum);
    printf("Values: a=%d, b=%d, c=%d, d=%d, e=%d, f=%d\n", a, b, c, d, e, f);
    
    /* Use results to affect return value */
    return (checksum & 255);
}
