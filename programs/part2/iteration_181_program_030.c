#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to create instruction patterns that trigger
 * GCC's delay slot filling logic in the reorg pass, specifically targeting
 * the uncovered lines 2135-2149 in reorg.cc
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization and ensure runtime values */
    volatile int init_val = argc;
    
    /* Declare multiple sets of variables to ensure resource independence */
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    int x = 0, y = 0, z = 0;
    int p = 0, q = 0, r = 0;
    
    /* Initialize with runtime-dependent values to prevent constant folding */
    a = init_val + 1;
    b = init_val * 2;
    c = init_val | 0x7F;
    d = init_val ^ 0x55;
    e = init_val & 0x3F;
    f = init_val + 7;
    
    x = argc * 3;
    y = argc + 11;
    z = argc - 5;
    
    p = argc << 2;
    q = argc >> 1;
    r = argc * argc;
    
    /* Loop provides scheduling context and prevents elimination */
    int iterations = (argc > 1) ? 100 : 200;
    
    for (int i = 0; i < iterations; ++i) {
        /* 
         * Create a conditional jump that depends on runtime values.
         * The condition uses modulo with a prime number to ensure
         * it's not optimized away and creates real branches.
         */
        if ((i + argc) % 13 == 0) {
            /* 
             * This should generate a simple conditional jump (simplejump_p).
             * The goto target is placed immediately before a safe instruction.
             */
            goto target_label;
        }
        
        /* Some computations to prevent the loop from being optimized away */
        x = y + z;
        y = x ^ p;
        z = q - r;
        
        continue;
        
        /* 
         * TARGET LABEL: The instruction immediately after this label
         * should be a candidate for delay slot filling.
         * Requirements:
         * 1. Not a jump
         * 2. Single RTL pattern (not SEQUENCE)
         * 3. Doesn't reference or set critical resources (&set, &needed)
         * 4. Not potentially trapping
         * 5. Eligible for delay slot
         */
    target_label:
        /* Safe, non-trapping arithmetic operation using different variables */
        a = b + c;  /* Candidate for delay slot filling */
        
        /* Additional operations to ensure this isn't a basic block with just one instruction */
        d = e ^ f;
        p = q & r;
        
        /* More computations to create register pressure and scheduling opportunities */
        x = (y << 2) | (z >> 1);
        y = (p + q) * 2;
        
        /* Another conditional to create additional control flow */
        if ((i * argc) % 7 == 0) {
            e = f + a;
        } else {
            f = e - b;
        }
    }
    
    /* 
     * Create observable side effects to prevent dead code elimination.
     * Compute a checksum using all modified variables.
     */
    int checksum = a + b + c + d + e + f + x + y + z + p + q + r;
    
    /* Print result to create observable behavior */
    printf("Result: %d (argc=%d)\n", checksum, argc);
    
    /* Additional loop to increase scheduling complexity */
    for (int j = 0; j < argc * 10; ++j) {
        /* Mix of operations to create various instruction types */
        a = b ^ j;
        b = c + (j & 0xF);
        c = d | (j << 1);
        
        /* Another conditional jump opportunity */
        if ((j + argc) % 11 == 3) {
            d = e + f;
        } else {
            d = e - f;
        }
        
        /* Bitwise operations are safe and non-trapping */
        p = q & (j * 2);
        q = r | (j + 1);
        r = p ^ q;
    }
    
    /* Final computation and output */
    int final_result = (a * b) + (c * d) - (e * f) + (x * y) - (z * p) + (q * r);
    printf("Final: %d\n", final_result);
    
    return final_result % 256;
}
