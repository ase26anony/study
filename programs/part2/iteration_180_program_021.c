/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_reorg test_reorg.c */
/* For MIPS architecture: gcc -O3 -march=mips64 -mtune=mips64 -o test_reorg test_reorg.c */

#include <stdio.h>
#include <stdlib.h>

/* Function containing the critical pattern */
int process_values(int iterations, int seed) {
    volatile int a = seed + 1;      /* Prevent constant propagation */
    volatile int b = seed + 2;
    volatile int c = seed + 3;
    volatile int d = seed + 4;
    int result = 0;
    
    /* Loop to create scheduling context */
    for (int i = 0; i < iterations; ++i) {
        /* Runtime-dependent condition to prevent optimization */
        if ((i + seed) % 7 == 0) {
            /* Simple conditional jump that should become a simplejump_p */
            if (a > b) {
                /* Jump to label where candidate instruction resides */
                goto target_label;
            }
        }
        
        /* Some computations to create register pressure */
        c = d ^ i;
        d = a + i;
        
        /* Continue normal flow */
        a = b + c;
        b = c - d;
        continue;
        
        /* Target label with simple, safe instruction */
        target_label:
        /* Candidate instruction for delay slot filling:
           Simple arithmetic, no trapping, no resource conflicts */
        a = b + c;  /* Should compile to simple register operation */
        
        /* Additional instruction to ensure target isn't isolated */
        b = c * d;
    }
    
    /* Use results to prevent dead code elimination */
    result = a ^ b ^ c ^ d;
    return result;
}

/* Second function with different pattern */
int alternate_pattern(int limit, int mod) {
    int x = 1, y = 2, z = 3, w = 4;
    int temp;
    
    for (int i = 0; i < limit; ++i) {
        /* Another conditional jump pattern */
        if (i % mod == 0) {
            if (x != y) {
                goto compute_point;
            }
        }
        
        x = y + z;
        y = z - w;
        z = w ^ i;
        continue;
        
        compute_point:
        /* Another candidate instruction - register move/add pattern */
        w = x + y;  /* Simple, non-trapping */
        
        /* Follow-up instruction */
        z = w >> 2;
    }
    
    return x + y + z + w;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int seed = 0;
    
    /* Use command line arguments to create runtime variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 100;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    /* Call functions with different patterns */
    int result1 = process_values(iterations, seed);
    int result2 = alternate_pattern(iterations * 2, (seed % 5) + 3);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Return non-deterministic result */
    return (result1 ^ result2) & 0xFF;
}
