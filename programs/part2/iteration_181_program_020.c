#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* 
     * Use volatile to prevent constant folding and ensure runtime values
     * are used for condition checks
     */
    volatile int init_val = argc;
    
    /* 
     * Declare multiple sets of variables to ensure resource independence:
     * - Variables for jump condition (x, y, z)
     * - Variables for target instruction (a, b, c)
     * - Variables for post-label operations (d, e, f)
     */
    int x = 0, y = 0, z = 0;
    int a = 0, b = 0, c = 0;
    int d = 0, e = 0, f = 0;
    int result = 0;
    
    /* Initialize with argc-dependent values to prevent optimization */
    x = init_val * 3 + 1;
    y = init_val * 7 + 2;
    z = init_val * 11 + 3;
    
    a = init_val * 13 + 4;
    b = init_val * 17 + 5;
    c = init_val * 19 + 6;
    
    d = init_val * 23 + 7;
    e = init_val * 29 + 8;
    f = init_val * 31 + 9;
    
    /*
     * Create a loop to provide scheduling context and prevent elimination.
     * The loop count depends on argc to prevent compile-time optimization.
     */
    int loop_count = (argc > 1) ? 100 : 200;
    
    for (int i = 0; i < loop_count; ++i) {
        /* 
         * Create a runtime-dependent condition that can't be optimized away.
         * This ensures we get a real conditional jump.
         */
        if ((i + init_val) % 13 == 0) {
            /* 
             * This is the critical jump that should become a simplejump_p.
             * The goto target is immediately before a safe instruction.
             */
            goto target_label;
        }
        
        /* Some computations to prevent the loop from being optimized away */
        x = y ^ z;
        y = z + i;
        z = x * 3;
        
        /* Skip the target instruction when not jumping */
        a = b - c;
        continue;
        
        /* 
         * TARGET LABEL: The instruction immediately after this label
         * should be the candidate for delay slot filling (next_trial).
         * It must be a simple, safe, non-jump instruction.
         */
    target_label:
        /* 
         * Safe instruction: register-to-register operation.
         * Uses different variables than the jump condition to avoid
         * resource conflicts with &set and &needed.
         * This is a simple addition - no trapping, no special registers.
         */
        a = b + c;
        
        /* 
         * Additional operations after the target to ensure it's not
         * the only instruction in its basic block.
         */
        d = e ^ f;
        e = f + 1;
        f = d * 2;
        
        /* More computations to create non-trivial control flow */
        if ((i % 7) == 0) {
            x = y - z;
        } else {
            x = y + z;
        }
    }
    
    /* 
     * Use all modified variables to create observable side effects.
     * This prevents dead code elimination.
     */
    result = a + b + c + d + e + f + x + y + z;
    
    /* Print result to prevent entire program from being optimized away */
    printf("Result: %d\n", result);
    
    return result % 256;
}
