/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_delay test_delay.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o test_delay test_delay.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and complex control flow */
int process_values(int argc, char **argv) {
    volatile int seed = argc;  /* Prevent constant folding */
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Create a loop to provide scheduling context */
    int iterations = (argc > 1) ? 100 : 200;
    
    for (int i = 0; i < iterations; ++i) {
        /* Use runtime-dependent condition to prevent optimization */
        if ((i + seed) % 7 == 0) {
            /* This is the critical conditional jump */
            /* The compiler should generate a simplejump_p to target_label */
            goto target_label;
        }
        
        /* Some computations to create register pressure */
        a = b + c;
        b = c * d;
        c = d - a;
        d = a ^ b;
        
        continue;  /* Ensure the label is not the only thing after jump */
        
    target_label:
        /* This instruction should be the candidate for delay slot filling */
        /* It's a simple, safe arithmetic operation that doesn't trap */
        a = b + c;  /* Simple addition - no trapping, no special registers */
        
        /* Additional operations to ensure target isn't isolated */
        b = c * d;
        c = d - a;
        
        /* Prevent loop unrolling from changing the pattern */
        if (i % 3 == 0) {
            d = a ^ b;
        } else {
            d = b ^ c;
        }
    }
    
    /* Use the modified variables to create observable side effects */
    result = a + b + c + d;
    
    /* Mix in some more control flow to prevent over-optimization */
    if (seed % 2 == 0) {
        result *= 2;
    } else {
        result /= 2;
    }
    
    return result;
}

/* Another function to increase compilation complexity */
void helper_func(int *x, int *y, int n) {
    for (int i = 0; i < n; i++) {
        x[i] = y[i] * 2;
        if (x[i] > 100) {
            x[i] = 100;
        }
    }
}

int main(int argc, char **argv) {
    int values[10];
    int values2[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Call helper to create more instruction variety */
    helper_func(values, values2, 10);
    
    /* Process with our delay-slot-triggering function */
    int result = process_values(argc, argv);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Also use array values */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += values[i];
    }
    printf("Array sum: %d\n", sum);
    
    return (result + sum) > 100 ? 0 : 1;
}
