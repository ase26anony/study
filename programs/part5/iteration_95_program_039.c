#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITER 5

/* Prevent inlining to ensure the loop structure remains intact */
__attribute__((noinline))
unsigned long long modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent optimization of outer loop */
    unsigned long long total_sum = 0;
    int i;
    
    /* Outer loop with volatile control to prevent unrolling */
    for (outer_counter = 0; outer_counter < OUTER_ITER; outer_counter++) {
        unsigned long long sum = 0;
        int prev_val = 0;
        
        /* Create control flow variability in outer loop */
        if (rand() % 2) {
            /* Inner loop with multiple dependencies and operations */
            for (i = 0; i < size; i++) {
                /* Multiple loads creating scheduling pressure */
                int a_val = a[i];
                int b_val = b[i];
                
                /* Operation with higher latency (multiplication) */
                int product = a_val * b_val;
                
                /* Loop-carried dependency: sum depends on previous iteration */
                sum = sum + product;
                
                /* Another operation with different latency (bitwise) */
                int masked = product & 0xFF;
                
                /* Distance-1 dependency: a[i] depends on a[i-1] */
                if (i > 0) {
                    /* Create distance-1 use case */
                    int prev_a = a[i-1];
                    /* Multiple uses of same value to trigger distance1_uses */
                    int temp1 = prev_a + masked;
                    int temp2 = prev_a - masked;
                    a[i] = (temp1 + temp2) / 2;  /* Complex update */
                } else {
                    a[i] = masked;
                }
                
                /* Additional operations to create more scheduling nodes */
                int shifted = product >> 3;
                int xored = shifted ^ b_val;
                sum += xored & 1;  /* Small addition to sum */
                
                /* Another distance-0 dependency within iteration */
                int intermediate = a_val + b_val;
                sum += intermediate % 256;
            }
        } else {
            /* Alternative path to create control flow */
            for (i = 0; i < size; i++) {
                int diff = a[i] - b[i];
                sum += diff * diff;
                a[i] = (a[i] + b[i]) / 2;
            }
        }
        
        total_sum += sum;
        
        /* Additional computation to prevent loop simplification */
        for (i = 0; i < 10; i++) {
            total_sum ^= (total_sum << 1) | 1;
        }
    }
    
    return total_sum;
}

int main() {
    int a[SIZE];
    int b[SIZE];
    int i;
    
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random data */
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the function with the complex loop */
    unsigned long long result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %llu\n", result);
    
    /* Additional print to use arrays */
    printf("First few a[] values: %d %d %d\n", a[0], a[1], a[2]);
    printf("First few b[] values: %d %d %d\n", b[0], b[1], b[2]);
    
    return 0;
}
