/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_reorg test_reorg.c */
/* For MIPS: mips64-linux-gnu-gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o test_reorg test_reorg.c */

#include <stdio.h>
#include <stdlib.h>

/* Function containing the critical pattern */
int process_values(int iterations, int seed) {
    volatile int a = seed + 1;  /* Prevent constant folding */
    int b = seed * 2;
    int c = seed / 3;
    int d = seed - 5;
    int result = 0;
    
    /* Loop to create scheduling context */
    for (int i = 0; i < iterations; ++i) {
        /* Create runtime-dependent condition to prevent optimization */
        if ((i + seed) % 7 == 0) {
            /* This should become a simple conditional jump to label */
            if (a > b && c != d) {
                /* The goto creates a simplejump_p to the label */
                goto target_label;
            }
        }
        
        /* Some intermediate computations */
        b = c + i;
        c = d ^ i;
        d = a | b;
        
        /* Continue after the label block */
        continue;
        
        /* TARGET LABEL - This instruction should be eligible for delay slot */
        /* It's a simple non-trapping instruction that doesn't conflict with jump resources */
        target_label:
        a = b + c;  /* Simple arithmetic - safe for delay slot */
        
        /* Additional instruction after target to ensure it's not alone */
        d = c * 2;
    }
    
    /* Use all variables to prevent dead code elimination */
    result = (a ^ b) | (c & d);
    return result;
}

/* Another function with similar pattern to increase chances */
int alternate_pattern(int limit, int mod) {
    int x = limit;
    int y = mod;
    int z = x * y;
    int w = y - x;
    
    for (int j = 0; j < limit; ++j) {
        /* Different condition pattern */
        if (j % 11 == mod) {
            if (x != y && z > w) {
                goto alt_target;
            }
        }
        
        x = y + j;
        y = z - j;
        z = w * j;
        continue;
        
        alt_target:
        w = x + y;  /* Another candidate for delay slot */
        z = w >> 2;
    }
    
    return x + y + z + w;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int seed = 42;
    
    /* Use command line arguments to create runtime-dependent values */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 100;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    /* Call functions with the patterns */
    int result1 = process_values(iterations, seed);
    int result2 = alternate_pattern(iterations * 2, seed % 7);
    
    /* Create observable output to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Use results to affect return value */
    return (result1 + result2) > 0 ? 0 : 1;
}
