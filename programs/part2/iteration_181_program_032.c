#include <stdio.h>
#include <stdlib.h>

/* 
 * This program is designed to create instruction patterns that trigger
 * GCC's delay slot filling logic in reorg.cc, specifically targeting
 * the uncovered lines 2135-2149.
 */

int main(int argc, char **argv) {
    /* Use volatile to prevent constant folding */
    volatile int init_val = argc;
    
    /* Declare multiple sets of variables to ensure resource independence */
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    int x = 0, y = 0, z = 0, w = 0, v = 0, u = 0;
    
    /* Initialize with runtime-dependent values */
    a = init_val + 1;
    b = init_val + 2;
    c = init_val + 3;
    d = init_val + 4;
    e = init_val + 5;
    f = init_val + 6;
    x = init_val + 7;
    y = init_val + 8;
    z = init_val + 9;
    w = init_val + 10;
    v = init_val + 11;
    u = init_val + 12;
    
    /* Loop to provide scheduling context and prevent elimination */
    int iterations = (argc > 1) ? 100 : 200;
    int checksum = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* 
         * Create a conditional jump that depends on runtime values.
         * The modulo operation prevents optimization.
         */
        if ((i + init_val) % 13 == 0) {
            /* 
             * This should generate a simple conditional jump (simplejump_p).
             * The goto target is immediately before a safe instruction.
             */
            goto target_label;
        }
        
        /* Some computations to create register pressure */
        x = y ^ z;
        w = v | u;
        checksum += (x + w) & 0xFF;
        
        /* Skip the target instruction when not jumping */
        continue;
        
    target_label:
        /*
         * This is the candidate instruction for delay slot filling (next_trial).
         * It must be:
         * 1. A non-jump instruction
         * 2. Not a SEQUENCE pattern
         * 3. Not referencing or setting critical resources (&set, &needed)
         * 4. Not potentially trapping
         * 5. Eligible for delay slot filling
         */
        a = b + c;  /* Simple arithmetic - safe and non-trapping */
        
        /* Additional operations to ensure this isn't a single-instruction block */
        d = e ^ f;
        checksum += (a + d) & 0xFF;
        
        /* More computations to prevent optimization */
        x = y - z;
        w = v & u;
    }
    
    /* 
     * Additional loop with different pattern to increase chances
     * of the reorg pass seeing the pattern
     */
    for (int j = 0; j < iterations / 2; ++j) {
        /* Different condition to create another potential jump */
        if ((j * init_val) % 17 == 0) {
            goto second_target;
        }
        
        /* More computations */
        a = b - c;
        checksum += a & 0xFF;
        continue;
        
    second_target:
        /* Another candidate instruction - register move pattern */
        x = y;  /* Simple move - often eligible for delay slots */
        
        /* Follow-up computations */
        z = w + v;
        checksum += (x + z) & 0xFF;
    }
    
    /* Use the results to prevent dead code elimination */
    checksum += a + b + c + d + e + f + x + y + z + w + v + u;
    
    /* Print to create observable side effect */
    printf("Result: %d\n", checksum);
    
    return checksum & 1;
}
