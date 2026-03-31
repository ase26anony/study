/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_reorg test_reorg.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o test_reorg test_reorg.c */

#include <stdio.h>
#include <stdlib.h>

/* Helper to prevent optimization */
static int volatile_sink;

/* Function containing the critical pattern */
int process_values(int argc, char **argv) {
    /* Use argc to create runtime-dependent values */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Variables for the target instruction */
    int a = 0, b = base + 1, c = base + 2, d = base + 3;
    int result = 0;
    
    /* Loop to provide scheduling context */
    int iterations = (argc > 2) ? 100 : 200;
    for (int i = 0; i < iterations; ++i) {
        /* Create a non-trivial condition that's not always true/false */
        if ((i + base) % 7 == 0) {
            /* Simple conditional jump - should become simplejump_p */
            if (d > c) {
                /* Jump to label where candidate instruction resides */
                goto target_label;
            }
        }
        
        /* Some intermediate computation */
        a = b + d;
        continue_after_label:
        c = a ^ b;  /* XOR is safe, non-trapping */
        d = c + i;
        
        /* Skip the target block when not jumping */
        if (0) {
            target_label:
            /* CRITICAL: This is the candidate instruction (next_trial) */
            /* Simple, safe arithmetic that doesn't trap */
            a = b + c;  /* Should compile to simple register operation */
            
            /* Additional instruction to ensure not the only one in block */
            b = c * d;  /* Multiplication is safe with small values */
            goto continue_after_label;
        }
        
        /* More computations to use variables and prevent dead code elimination */
        result += a - b + c * d;
    }
    
    /* Create observable side-effect */
    volatile_sink = result;
    
    /* Use all variables to prevent optimization */
    return a + b + c + d + result;
}

/* Main function with command-line arguments */
int main(int argc, char **argv) {
    int ret = process_values(argc, argv);
    
    /* Print to prevent optimization and verify execution */
    printf("Result: %d (volatile sink: %d)\n", ret, volatile_sink);
    
    return (ret > 0) ? 0 : 1;
}
