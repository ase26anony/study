/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test test.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -o test test.c */

#include <stdio.h>
#include <stdlib.h>

/* Function containing the critical pattern */
int process_values(int argc, char **argv) {
    volatile int trigger = argc; /* Prevent constant folding */
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Loop to provide scheduling context */
    for (int i = 0; i < (argc > 1 ? 100 : 200); ++i) {
        /* Mix of operations to create register pressure */
        int temp = b * c;
        d = temp - a;
        
        /* Critical pattern: conditional jump with potential delay slot candidate */
        if ((i % 7) == (trigger & 3)) { /* Runtime-dependent condition */
            /* Jump to label where candidate instruction resides */
            goto target_label;
        }
        
        /* Some other code to prevent optimization */
        a = b + d;
        continue;
        
    target_label:
        /* Candidate instruction for delay slot filling */
        /* Simple, safe arithmetic that doesn't trap */
        a = b + c;  /* This should be the 'next_trial' instruction */
        
        /* Additional instruction to ensure not the only one in block */
        b = c * d;
        
        /* Continue loop */
        result += a;
    }
    
    /* Use variables to prevent dead code elimination */
    result += a + b + c + d;
    
    /* Create observable side effect */
    printf("Result: %d\n", result);
    return result;
}

/* Additional function to create more complex control flow */
void helper(int *x, int *y) {
    /* Simple operations that won't interfere with main pattern */
    *x = *y + 1;
    *y = *x - 1;
}

int main(int argc, char **argv) {
    int x = 10, y = 20;
    
    /* Call helper to add complexity to control flow graph */
    helper(&x, &y);
    
    /* Main processing with the critical pattern */
    int result = process_values(argc, argv);
    
    /* Additional computation using helper-modified values */
    result += x + y;
    
    return result % 256;
}
