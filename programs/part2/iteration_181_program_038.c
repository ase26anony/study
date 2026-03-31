#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* 
     * Use volatile to prevent constant folding and ensure variables
     * remain live and distinct for resource tracking
     */
    volatile int a = argc + 1;
    volatile int b = argc + 2;
    volatile int c = argc + 3;
    volatile int d = argc + 4;
    volatile int e = argc + 5;
    volatile int f = argc + 6;
    
    /* Additional variables to ensure distinct register sets */
    int x = 0, y = 0, z = 0;
    int r1 = 0, r2 = 0, r3 = 0;
    
    /* 
     * Loop provides scheduling context and prevents elimination.
     * The loop count depends on argc to prevent compile-time optimization.
     */
    int iterations = (argc > 1) ? 100 : 200;
    
    for (int i = 0; i < iterations; ++i) {
        /* 
         * First block: computations that set up values for the jump condition
         * Use distinct variables from the target instruction's computation
         */
        x = i * 3;
        y = argc * 7;
        z = x ^ y;
        
        /* 
         * KEY CONSTRUCT: Conditional jump that should be recognized as simplejump_p
         * The condition uses runtime values to prevent optimization
         */
        if ((z + i) % 13 == 0) {
            /* 
             * This goto creates a simple conditional jump to a label.
             * The jump uses no resources that conflict with the target instruction.
             */
            goto target_label;
        }
        
        /* 
         * Alternative path to ensure the jump isn't always taken
         * and to create more complex control flow
         */
        r1 = r2 + r3;
        r2 = r1 ^ i;
        r3 = r2 & 0xFF;
        
        /* Skip the target instruction when not jumping */
        continue;
        
    target_label:
        /* 
         * TARGET INSTRUCTION: This is the candidate for delay slot filling (next_trial).
         * It must be:
         * 1. A non-jump instruction
         * 2. Not a SEQUENCE pattern
         * 3. Not referencing or setting resources in &set or &needed
         * 4. Not potentially trapping
         * 5. Eligible for delay slot filling
         * 
         * Using simple arithmetic with variables not live across the jump
         */
        a = b + c;  /* Simple addition, no side effects, not trapping */
        
        /* 
         * Additional instruction after the label to ensure the target
         * isn't the only instruction in its basic block
         */
        d = e ^ f;
        
        /* Continue loop execution */
        r1 = a + d;
    }
    
    /* 
     * Use the computed values to create observable side effects
     * This prevents dead code elimination
     */
    int checksum = a + b + c + d + e + f + x + y + z + r1 + r2 + r3;
    
    /* Print result to ensure values are used */
    printf("Result: %d (argc=%d)\n", checksum, argc);
    
    return checksum & 0xFF;
}
