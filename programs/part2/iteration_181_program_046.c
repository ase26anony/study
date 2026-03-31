#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is crafted to trigger delay slot filling logic in GCC's reorg pass.
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
     * Declare distinct sets of variables to ensure resource independence:
     * - Variables for the jump condition (x, y, z)
     * - Variables for the target instruction (a, b, c)
     * - Additional variables for post-label operations (d, e, f)
     */
    int x, y, z;
    int a = 0, b = 0, c = 0;
    int d = 0, e = 0, f = 0;
    
    /* Initialize with runtime-dependent values */
    x = init_val;
    y = argc * 3;
    z = argc + 7;
    
    a = argc * 2;
    b = argc + 5;
    c = argc * 4;
    
    d = argc + 1;
    e = argc * 6;
    f = argc + 9;
    
    /*
     * Use a loop to provide scheduling context and prevent elimination.
     * The loop count depends on argc to prevent compile-time optimization.
     */
    int loop_count = (argc > 1) ? 100 : 200;
    
    for (int i = 0; i < loop_count; ++i) {
        /*
         * Create a runtime-dependent condition that can't be optimized away.
         * Using modulo with a non-power-of-two divisor prevents optimization.
         */
        int condition = (i + argc) % 13;
        
        /*
         * The key construct: a conditional jump to a label.
         * This should generate a simplejump_p instruction.
         */
        if (condition == 0) {
            /* 
             * Use goto to create an explicit jump to label.
             * This is more likely to create the simple jump pattern we need.
             */
            goto target_label;
        }
        
        /*
         * Some computations to prevent the compiler from optimizing
         * the entire loop structure away
         */
        x = y + z;
        y = z - i;
        z = x ^ y;
        
        /* Skip the target instruction when not jumping */
        continue;
        
    target_label:
        /*
         * TARGET INSTRUCTION CANDIDATE for delay slot filling.
         * This must be a simple, safe, non-jump instruction that:
         * 1. Is not a SEQUENCE pattern
         * 2. Doesn't reference or set critical resources (&set, &needed)
         * 3. Is not potentially trapping
         * 4. Is eligible for delay slot filling
         * 
         * Using simple arithmetic with distinct variables that aren't
         * live across the jump.
         */
        a = b + c;  /* Simple register-to-register operation */
        
        /*
         * Additional operations after the label to ensure the target
         * instruction isn't the only one in its basic block
         */
        d = e ^ f;  /* Bitwise operation - safe and non-trapping */
        f = d | 0x7F; /* Another safe operation */
        
        /* Continue loop execution */
        continue;
    }
    
    /*
     * Use the modified variables to create observable side effects.
     * This prevents dead code elimination of the entire computation.
     */
    int checksum = a + b + c + d + e + f + x + y + z;
    
    /* 
     * Print the checksum to create observable output.
     * Use volatile to ensure the computation isn't optimized away.
     */
    volatile int result = checksum;
    printf("Result: %d (argc=%d)\n", result, argc);
    
    return 0;
}
