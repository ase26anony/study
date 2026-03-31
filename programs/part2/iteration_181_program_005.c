#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger delay slot filling logic in GCC's reorg pass.
 * It creates a conditional jump to a label where the target instruction is a safe,
 * non-jump candidate for moving into the delay slot.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization of variables */
    volatile int init_val = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Distinct sets of variables to avoid resource conflicts */
    int a = 0, b = 0, c = 0;      /* Used in jump condition */
    int d = 0, e = 0, f = 0;      /* Used at target label */
    int g = 0, h = 0, i = 0;      /* Used after label */
    int result = 0;
    
    /* Initialize with runtime-dependent values */
    a = init_val + 1;
    b = init_val * 2;
    c = init_val / 3 + 1;  /* Ensure non-zero */
    
    d = init_val + 10;
    e = init_val + 20;
    f = init_val + 30;
    
    g = init_val + 40;
    h = init_val + 50;
    i = init_val + 60;
    
    /* Loop to provide scheduling context and prevent elimination */
    int loop_count = (argc > 2) ? 100 : 200;
    for (int iter = 0; iter < loop_count; ++iter) {
        /* Create runtime-dependent condition to prevent constant folding */
        int condition = (iter + argc) % 13;
        
        /* 
         * Key construct: conditional jump to label
         * The condition uses variables a, b, c to create non-trivial computation
         */
        if (((a + b - c) & 0xF) == condition) {
            /* 
             * This should generate a simple conditional jump (simplejump_p)
             * The target is safe_label where a safe instruction follows
             */
            goto safe_label;
        }
        
        /* Some computation to prevent the block from being empty */
        g = h ^ i;
        h = g + 1;
        i = h * 2;
        
        continue;
        
        /* 
         * Target label with safe instruction candidate for delay slot
         * The instruction at the label must:
         * 1. Not be a jump
         * 2. Not reference or set critical resources (&set, &needed)
         * 3. Not be potentially trapping
         * 4. Be a single RTL pattern (not SEQUENCE)
         */
    safe_label:
        /* Safe, non-trapping arithmetic operation using different variables */
        d = e + f;  /* Simple addition - safe for delay slot */
        
        /* Additional operations to ensure this isn't the only instruction */
        g = d & 0xFF;
        h = g | 0x55;
        i = h ^ 0xAA;
    }
    
    /* 
     * Use all variables to create observable side effects
     * This prevents dead code elimination
     */
    result = a + b + c + d + e + f + g + h + i;
    
    /* Mix results to create checksum */
    int checksum = result ^ (argc * 12345);
    
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    /* Additional loop to increase scheduling opportunities */
    for (int j = 0; j < (argc * 10); ++j) {
        /* More safe operations that could be scheduled */
        a = b & c;
        b = c | d;
        c = d ^ e;
        d = e + f;
        e = f - g;
        f = g * 2;  /* Multiplication by 2 is safe (shift) */
        
        /* Another conditional jump opportunity */
        if ((j & 3) == (argc & 3)) {
            goto another_label;
        }
        
        g = h + i;
        h = i - a;
        i = a & b;
        
        continue;
        
    another_label:
        /* Another safe candidate instruction */
        a = b + c;
        
        /* Follow-up operations */
        d = e ^ f;
        g = h | i;
    }
    
    /* Final computation using all variables */
    result += a * 3 + b * 5 + c * 7 + d * 11 + e * 13 + f * 17 + g * 19 + h * 23 + i * 29;
    
    printf("Final result: %d\n", result);
    
    return result & 0xFF;
}
