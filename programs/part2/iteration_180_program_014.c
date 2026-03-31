/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o delay_slot_test delay_slot_test.c */
/* For MIPS: mips64-linux-gnu-gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o delay_slot_test delay_slot_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and prevent optimization */
int fill_delay_slots_test(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent constant propagation */
    volatile int a = 1;
    volatile int b = 2;
    volatile int c = 3;
    volatile int d = 4;
    int e = 5, f = 6, g = 7, h = 8;
    
    /* Use argc to create runtime-dependent loop bounds */
    int iterations = (argc > 1) ? 100 : 200;
    
    /* Loop to provide scheduling context */
    for (int i = 0; i < iterations; ++i) {
        /* Create a non-trivial condition that can't be optimized away */
        int condition = (i * 13 + argc) % 7;
        
        /* The key construct: conditional jump to label */
        if (condition == 0) {
            /* Use goto to create a simplejump_p pattern */
            goto target_label;
        }
        
        /* Some intermediate computations to create register pressure */
        e = f + g;
        f = g ^ h;
        g = h << 2;
        h = e - f;
        
        continue;
        
        /* Target label with simple, safe instruction */
        target_label:
        /* This is the candidate for delay slot filling */
        /* Simple arithmetic that doesn't trap and uses different registers */
        a = b + c;
        
        /* Additional operations to ensure target isn't isolated */
        b = c * d;
        c = d | 0xFF;
        d = a ^ b;
    }
    
    /* Post-loop computations to create observable side effects */
    int result = a + b + c + d + e + f + g + h;
    
    /* Use result to prevent dead code elimination */
    if (result > 1000) {
        printf("Result: %d\n", result);
    }
    
    return result;
}

/* Second function with different pattern to increase coverage chances */
int alternative_pattern(int x, int y) {
    volatile int p = x;
    volatile int q = y;
    int r = 0, s = 0;
    
    /* Different loop structure */
    for (int j = 0; j < 50; j++) {
        /* Another conditional jump pattern */
        if ((j + x) % 5 == 2) {
            goto alt_target;
        }
        
        r = p + q;
        s = p - q;
        p = r ^ s;
        q = s << 1;
        
        continue;
        
        alt_target:
        /* Another safe candidate instruction */
        p = q + 1;
        
        /* Follow-up instructions */
        q = p * 2;
        r = q >> 1;
    }
    
    return p + q + r + s;
}

/* Main function with command line arguments */
int main(int argc, char **argv) {
    int result1 = fill_delay_slots_test(argc, argv);
    int result2 = alternative_pattern(argc, result1);
    
    /* Final computation to use all results */
    int final_result = result1 ^ result2;
    
    /* Print to create observable output */
    if (argc > 1) {
        printf("Final: %d\n", final_result);
    }
    
    return final_result % 256;
}
