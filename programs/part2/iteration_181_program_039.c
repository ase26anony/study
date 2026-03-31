#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is crafted to trigger specific delay slot filling logic
 * in GCC's reorg.cc during compilation with optimization.
 * The goal is to create a conditional jump to a label where the
 * instruction at the target is safe to move into the delay slot.
 */

int main(int argc, char **argv) {
    /* 
     * Use volatile to prevent constant folding and ensure
     * the values remain runtime-dependent
     */
    volatile int init_val = argc;
    
    /* 
     * Two distinct sets of variables to ensure resource independence:
     * Set 1: Used for the jump condition
     * Set 2: Used at the target label (candidate for delay slot)
     */
    int a = 0, b = 0, c = 0;      // Set 1 - jump condition variables
    int x = 0, y = 0, z = 0;      // Set 2 - target instruction variables
    int p = 0, q = 0, r = 0;      // Additional variables for post-label ops
    
    /* Initialize with argc-dependent values to prevent optimization */
    a = init_val + 1;
    b = init_val * 2;
    c = init_val | 0x7F;
    
    x = (init_val << 1) + 3;
    y = (init_val >> 1) ^ 0x55;
    z = init_val & 0x3F;
    
    p = init_val + 0x10;
    q = init_val - 0x20;
    r = init_val ^ 0xAA;
    
    /*
     * Loop provides scheduling context and prevents elimination
     * of the jump pattern
     */
    int iterations = (argc > 1) ? 100 : 200;
    for (int i = 0; i < iterations; ++i) {
        /* 
         * Complex condition to prevent optimization:
         * - Uses multiple variables
         * - Includes arithmetic and bitwise operations
         * - Result depends on loop counter and argc
         */
        int condition = (i + a) * b;
        condition = condition ^ c;
        
        /* 
         * The key construct: conditional jump to a label
         * The condition uses modulo with a non-power-of-two
         * to prevent optimization
         */
        if ((condition % 13) == ((i + argc) & 3)) {
            /* 
             * Use goto instead of structured control flow
             * to create a simple jump to label
             */
            goto target_label;
        }
        
        /* 
         * Some computation in the fall-through path
         * to make the CFG non-trivial
         */
        a = (a + i) & 0xFF;
        b = b ^ (i << 2);
        c = c + (i % 5);
        
        /* Skip the target label code in fall-through case */
        continue;
        
    target_label:
        /*
         * Target instruction candidate for delay slot filling:
         * - Simple arithmetic operation (safe, non-trapping)
         * - Uses different variables than the jump condition
         * - Single RTL pattern (not a SEQUENCE)
         * - Not a jump instruction
         */
        x = y + z;  // Candidate instruction for delay slot
        
        /*
         * Additional operations after the label to ensure
         * the target isn't the only instruction in its block
         */
        p = q ^ r;
        r = (p + 1) & 0x7F;
        
        /* 
         * More computations to create register pressure
         * and scheduling opportunities
         */
        y = (y + 1) % 256;  // Safe modulo with power of two
        z = z | 0x80;
    }
    
    /* 
     * Use all variables in final computation to prevent
     * dead code elimination
     */
    int checksum = a + b + c + x + y + z + p + q + r;
    
    /* 
     * Print result to create observable side effect
     * and prevent elimination of entire computation
     */
    printf("Result: %d (argc=%d)\n", checksum, argc);
    
    return checksum & 0xFF;
}
