/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test test.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o test test.c */

#include <stdio.h>
#include <stdlib.h>

/* Function containing the critical pattern for delay slot filling */
static int process_values(int argc, char **argv) {
    volatile int seed = argc; /* Prevent constant propagation */
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Loop to create scheduling context */
    for (int i = 0; i < (argc > 1 ? 100 : 200); ++i) {
        /* Mix up values to prevent dead code elimination */
        int temp = seed + i;
        
        /* Critical pattern: conditional jump with potential delay slot candidate */
        if ((temp % 7) == 0) {
            /* This goto creates a simplejump_p to target_label */
            goto target_label;
        }
        
        /* Some intermediate computation */
        d = a + b;
        continue;
        
        /* Target label with simple, safe instruction */
        target_label:
        /* Candidate for delay slot: simple arithmetic, no traps, no resource conflicts */
        a = b + c;  /* This should be eligible for delay slot filling */
        
        /* Follow-up instruction to ensure target isn't isolated */
        b = c * d;
        
        /* More computation to create register pressure */
        c = d - a;
    }
    
    /* Use results to prevent elimination */
    result = a + b + c + d;
    return result;
}

/* Wrapper to ensure function isn't inlined prematurely */
__attribute__((noinline)) 
static int compute_checksum(int argc, char **argv) {
    int sum = 0;
    
    /* Multiple iterations to increase scheduling opportunities */
    for (int j = 0; j < 3; ++j) {
        sum += process_values(argc + j, argv);
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int checksum = compute_checksum(argc, argv);
    
    /* Print result to create observable side effect */
    printf("Result: %d\n", checksum);
    
    return (checksum > 1000) ? 0 : 1;
}
