/* Program to trigger delay slot filling logic in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization */
static volatile int global_seed = 42;

/* Function with the key pattern for delay slot filling */
int process_values(int iterations, int threshold) {
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Loop to create scheduling context */
    for (int i = 0; i < iterations; ++i) {
        /* Mix values to create data dependencies */
        int temp = (a ^ b) + (c & d);
        
        /* Create runtime-dependent condition to prevent constant folding */
        if ((i + global_seed) % threshold == 0) {
            /* This is the critical conditional jump */
            /* The compiler should generate a simplejump_p to target_label */
            goto target_label;
        }
        
        /* Alternative path */
        a = b + c;
        b = c - d;
        continue;
        
        /* Target label with simple, safe instruction */
        /* This instruction should be eligible for delay slot filling */
        target_label:
        /* Simple arithmetic that doesn't trap and uses independent registers */
        d = a + b;  /* Candidate for delay slot - doesn't conflict with jump resources */
        
        /* Continue with other operations so target isn't isolated */
        c = d * 2;
        a = b ^ c;
    }
    
    /* Use results to prevent dead code elimination */
    result = (a + b) ^ (c - d);
    return result;
}

/* Second function with different pattern */
int alternate_pattern(int base, int mod) {
    int x = base, y = base + 1, z = base + 2;
    int sum = 0;
    
    for (int i = 0; i < 50; ++i) {
        /* Create varying condition */
        if ((x + i) % mod == (y % 3)) {
            /* Another conditional jump opportunity */
            goto compute_point;
        }
        
        x = y + z;
        y = z - i;
        continue;
        
        compute_point:
        /* Another candidate instruction - register move pattern */
        z = x + y;  /* Should be safe for delay slot */
        
        /* Follow-up operations */
        x = z >> 1;
        y = x * 3;
        
        sum += x + y + z;
    }
    
    return sum;
}

/* Main function with command-line control */
int main(int argc, char *argv[]) {
    int iterations = 100;
    int threshold = 7;
    
    /* Use command-line arguments to create runtime variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        threshold = atoi(argv[2]);
        if (threshold <= 0) threshold = 7;
    }
    
    /* Update global seed based on arguments */
    global_seed = (argc > 0) ? argc : 42;
    
    /* Call functions to generate the patterns */
    int result1 = process_values(iterations, threshold);
    int result2 = alternate_pattern(global_seed, threshold + 1);
    
    /* Combine results to create observable output */
    int final_result = result1 ^ result2;
    
    printf("Result: %d (0x%x)\n", final_result, final_result);
    
    /* Return non-constant result */
    return (final_result != 0) ? 0 : 1;
}
