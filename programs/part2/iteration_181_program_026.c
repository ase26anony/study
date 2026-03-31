#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is crafted to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a
 * safe, non-jump candidate for moving into the delay slot.
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
     * - Variables for post-label computation (d, e, f)
     */
    int x = 0, y = 0, z = 0;
    int a = 0, b = 0, c = 0;
    int d = 0, e = 0, f = 0;
    int result = 0;
    
    /* Initialize with runtime-dependent values */
    x = init_val * 3 + 1;
    y = init_val * 5 + 2;
    z = init_val * 7 + 3;
    
    a = init_val * 11 + 4;
    b = init_val * 13 + 5;
    c = init_val * 17 + 6;
    
    d = init_val * 19 + 7;
    e = init_val * 23 + 8;
    f = init_val * 29 + 9;
    
    /*
     * Loop provides scheduling context and prevents elimination
     * The loop count depends on argc to prevent compile-time optimization
     */
    int loop_count = (argc > 1) ? 100 : 200;
    
    for (int i = 0; i < loop_count; ++i) {
        /* 
         * Create a runtime-dependent condition for the jump
         * Using modulo with a prime number to create irregular pattern
         */
        if ((i + init_val) % 13 == 0) {
            /* 
             * This should generate a simple conditional jump (simplejump_p)
             * The goto prevents if-conversion to conditional moves
             */
            goto target_label;
        }
        
        /* Some computation to prevent dead code elimination */
        x = y + z;
        y = z - x;
        z = x * 2;
        
        /* Skip the target instruction when not jumping */
        continue;
        
        /* 
         * TARGET LABEL: The instruction here should be a candidate for delay slot
         * This must be a simple, non-jump instruction that doesn't reference
         * resources tracked in &set and &needed
         */
    target_label:
        /* 
         * Safe target instruction: simple arithmetic with distinct variables
         * Not using condition codes, stack pointer, or special registers
         * Variables a, b, c are not live across the jump
         */
        a = b + c;  /* This is the candidate instruction (next_trial) */
        
        /* 
         * Additional computation after the label ensures the target
         * instruction is not the only one in its basic block
         */
        d = e ^ f;  /* Bitwise operation, safe and non-trapping */
        
        /* More computations to create non-trivial control flow */
        result += (a * d) % 31;
    }
    
    /* 
     * Use all modified variables to create observable side effects
     * This prevents dead code elimination of the entire computation
     */
    int checksum = a + b + c + d + e + f + x + y + z + result;
    
    /* Print result to prevent complete optimization */
    printf("Result: %d (argc=%d)\n", checksum, argc);
    
    return checksum % 256;
}
