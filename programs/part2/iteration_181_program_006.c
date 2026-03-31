#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is crafted to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is
 * a safe, non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of variables */
    volatile int init_val = argc;
    
    /* Declare distinct sets of variables to avoid resource conflicts */
    int a = 0, b = 0, c = 0;      /* Used in jump condition */
    int d = 0, e = 0, f = 0;      /* Used at target label */
    int g = 0, h = 0, i = 0;      /* Used after target label */
    int j = 0, k = 0, l = 0;      /* Additional variables */
    
    /* Initialize with argc-dependent values to prevent compile-time optimization */
    a = init_val + 1;
    b = init_val + 2;
    c = init_val + 3;
    d = init_val + 4;
    e = init_val + 5;
    f = init_val + 6;
    g = init_val + 7;
    h = init_val + 8;
    i = init_val + 9;
    j = init_val + 10;
    k = init_val + 11;
    l = init_val + 12;
    
    /* Loop to provide scheduling context and prevent elimination */
    int loop_count = (argc > 1) ? 100 : 200;
    for (int iter = 0; iter < loop_count; ++iter) {
        /* 
         * Create a conditional jump that depends on runtime values.
         * The condition uses modulo to create unpredictable jumps.
         */
        if ((iter + argc) % 13 == 0) {
            /* 
             * This is the simple conditional jump (simplejump_p).
             * It jumps to a label where we have a safe instruction.
             */
            goto target_label;
        }
        
        /* Some computations to prevent the jump from being optimized away */
        a = b + c;
        b = c - a;
        c = a ^ b;
        
        continue;
        
        /* 
         * TARGET LABEL: The instruction here should be eligible for delay slot filling.
         * Requirements:
         * 1. Not a jump
         * 2. Not a SEQUENCE pattern
         * 3. Doesn't reference or set critical resources (&set, &needed)
         * 4. Not potentially trapping
         * 5. Eligible for delay slot
         */
        target_label:
            /* Safe, non-trapping arithmetic operation using distinct variables */
            d = e + f;      /* Candidate for delay slot filling */
            
            /* Additional operations to ensure this isn't a single-instruction block */
            g = h & i;
            h = j | k;
            i = l ^ g;
            
            /* Continue with loop */
            continue;
    }
    
    /* 
     * Use all variables to create observable side effects and prevent dead code elimination.
     * This ensures the instructions survive through the reorg pass.
     */
    int checksum = a + b + c + d + e + f + g + h + i + j + k + l;
    
    /* Print result to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    /* Additional volatile operations to affect scheduling */
    volatile int barrier = checksum;
    barrier = barrier + 1;
    
    return checksum % 256;
}
