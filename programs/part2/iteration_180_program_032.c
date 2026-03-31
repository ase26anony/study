/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_reorg test_reorg.c */
/* For MIPS architecture: gcc -O3 -march=mips64 -mtune=mips64 -o test_reorg test_reorg.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and prevent optimization */
int fill_delay_slot_pattern(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent constant folding */
    volatile int a = 0, b = 1, c = 2, d = 3;
    volatile int result = 0;
    volatile int *ptr = &result;
    
    /* Use argc to create runtime-dependent loop bounds */
    int limit = (argc > 1) ? 100 : 200;
    
    /* Loop to provide scheduling context */
    for (int i = 0; i < limit; ++i) {
        /* Create a condition that's not always true/false */
        if (i % 7 == 0) {
            /* Simple conditional jump - should become simplejump_p in RTL */
            if ((i * b) > (c * d)) {
                /* This goto creates a jump to label */
                goto target_label;
            }
        }
        
        /* Some computations to create register pressure */
        a = b + c;
        b = c ^ d;
        c = d - a;
        d = a | b;
        
        continue;
        
        /* Target label with a simple, safe instruction */
        target_label:
        /* This is the candidate for delay slot filling:
           - Simple arithmetic (addition)
           - No trapping (no division, no null dereference)
           - Uses different registers than the jump condition
           - Not a jump instruction */
        a = b + c;
        
        /* Continue with other operations so target isn't isolated */
        b = c * d;
        c = d + 1;
    }
    
    /* Use the variables to create observable side effects */
    result = a + b + c + d;
    
    /* Additional control flow to prevent dead code elimination */
    if (argc > 2) {
        result += atoi(argv[2]);
    }
    
    return result;
}

/* Second function with different pattern to increase coverage chances */
int alternative_pattern(int x, int y) {
    volatile int p = x, q = y, r = 0, s = 0;
    
    /* Nested loops for more complex control flow */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 20; j++) {
            /* Another conditional jump opportunity */
            if ((i * j) < (p * q)) {
                if (p > q) {
                    goto alt_target;
                }
            }
            
            p = q + i;
            q = p - j;
            
            continue;
            
            alt_target:
            /* Different simple instruction: bitwise operation */
            r = p & q;
            
            /* Follow with other operations */
            s = r | 0xFF;
            p = s ^ q;
        }
    }
    
    return p + q + r + s;
}

int main(int argc, char **argv) {
    int result1 = fill_delay_slot_pattern(argc, argv);
    int result2 = alternative_pattern(argc, argc * 2);
    
    /* Print results to prevent complete optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Use results in return value */
    return (result1 + result2) & 0xFF;
}
