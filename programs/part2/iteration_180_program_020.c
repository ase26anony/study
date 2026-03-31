/* This program is designed to trigger delay slot filling logic in GCC's reorg pass.
   It creates a conditional jump where the target instruction is safe to move into
   the delay slot. */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization from eliminating the conditional */
static volatile int always_zero = 0;

/* Function to create complex enough control flow */
int fill_delay_slot_test(int argc, char **argv) {
    /* Declare and initialize variables - use different registers */
    int a = 1, b = 2, c = 3, d = 4;
    int e = 5, f = 6, g = 7, h = 8;
    int result = 0;
    
    /* Use argc to create runtime-dependent values */
    int limit = (argc > 1) ? 100 : 200;
    int mod_base = (argc > 2) ? 7 : 11;
    
    /* Loop to provide scheduling context */
    for (int i = 0; i < limit; ++i) {
        /* Create a conditional that's not always true/false */
        if ((i % mod_base) == (argc % 3)) {
            /* This goto creates a simplejump_p to a label */
            goto target_label;
        }
        
        /* Some computation to prevent optimization */
        a = b + c;
        b = c * d;
        c = d ^ a;
        d = a + i;
        
        /* Skip the target code when not jumping */
        continue;
        
        /* TARGET LABEL: This instruction should be eligible for delay slot filling */
        target_label:
        /* Simple, safe instruction that doesn't trap or conflict with jump resources */
        e = f + g;  /* Register-to-register operation, no CC, no stack pointer */
        
        /* Continue with other operations so target isn't isolated */
        f = g * h;
        g = h ^ e;
        h = e + i;
        
        /* Use volatile to prevent dead code elimination */
        always_zero = 0;
    }
    
    /* Use the variables to create observable side effects */
    result = a + b + c + d + e + f + g + h;
    
    /* Mix with argc to prevent constant folding */
    return result ^ argc;
}

/* Second function with different pattern to increase coverage chances */
int alternative_pattern(int x, int y) {
    int p = x, q = y, r = 0, s = 0;
    
    /* Nested loop for more complex control flow */
    for (int i = 0; i < 50; ++i) {
        for (int j = 0; j < 20; ++j) {
            /* Conditional jump based on computation */
            if (((i * j) & 0xF) == (x & 0xF)) {
                goto alt_target;
            }
            
            p = q + i;
            q = p * j;
            continue;
            
            alt_target:
            /* Another safe candidate for delay slot */
            r = s + 1;  /* Simple increment, no resource conflicts */
            
            s = r ^ j;
        }
    }
    
    return p + q + r + s;
}

/* Main function to drive the test */
int main(int argc, char **argv) {
    int result1 = fill_delay_slot_test(argc, argv);
    int result2 = alternative_pattern(argc, result1);
    
    /* Print results to prevent complete optimization */
    printf("Results: %d %d\n", result1, result2);
    
    /* Return non-constant to prevent optimization */
    return (result1 + result2) & 0xFF;
}
