/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_delay test_delay.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o test_delay test_delay.c */

#include <stdio.h>
#include <stdlib.h>

/* Function with carefully structured control flow to create fillable delay slots */
int process_values(int iterations, int seed) {
    volatile int a = seed + 1;   /* Prevent constant propagation */
    int b = seed * 2;
    int c = seed / 3;
    int d = seed - 5;
    int e = 0;
    
    /* Loop to create scheduling context */
    for (int i = 0; i < iterations; ++i) {
        /* Create runtime-dependent condition to prevent optimization */
        if ((i + seed) % 7 == 0) {
            /* Simple conditional jump - target is the instruction after label */
            if (a > b && c != 0) {
                /* This goto creates a simplejump_p in RTL */
                goto target_label;
            }
        }
        
        /* Some computations to create register pressure */
        b = c + d;
        c = d ^ i;
        d = a - i;
        
        continue;
        
    target_label:
        /* CRITICAL: This instruction should be eligible for delay slot filling */
        /* Simple arithmetic, no trapping, no resource conflicts */
        a = b + c;  /* next_trial candidate */
        
        /* Additional instruction to ensure target isn't isolated */
        d = e * 2;
        
        /* More computations to prevent tail merging */
        e = (a & 0xFF) | (d & 0xFF00);
    }
    
    /* Use results to prevent dead code elimination */
    return a + b + c + d + e;
}

/* Second function with different pattern to increase coverage chances */
int alternate_pattern(int limit, int mod) {
    int x = limit;
    int y = mod * 2;
    int z = 3;
    int w = 0;
    
    for (int j = 0; j < limit; ++j) {
        /* Different condition pattern */
        if ((j & 3) == (mod & 3)) {
            if (x != y) {
                goto alt_target;
            }
        }
        
        /* Mix of operations */
        x = y + z;
        y = z - j;
        z = x | y;
        
        if (j == limit / 2) {
            w = x * y;
        }
        
        continue;
        
    alt_target:
        /* Another candidate for delay slot filling */
        w = x + y;  /* Simple, non-trapping operation */
        
        /* Follow-up operation */
        z = w >> 2;
        
        /* Bitwise operation - safe, no trapping */
        x = y ^ z;
    }
    
    return x + y + z + w;
}

int main(int argc, char *argv[]) {
    int result1, result2;
    
    /* Use argc for runtime-dependent values */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    int seed = argc;
    
    /* Call first pattern */
    result1 = process_values(iterations, seed);
    
    /* Call second pattern */
    result2 = alternate_pattern(iterations * 2, seed + 1);
    
    /* Combine results to create observable output */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Return non-zero for verification */
    return (result1 != 0 || result2 != 0) ? 0 : 1;
}
