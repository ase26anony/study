#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
long long modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    long long total_sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        long long sum = 0;
        int prev_val = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with complex dependencies */
        for (i = 0; i < size; i++) {
            /* Multiple operations with different latencies */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Integer multiply (higher latency) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency on sum */
            sum = sum + product;
            
            /* Additional arithmetic to create more dependencies */
            int temp = product & 0xFF;  /* Bitwise operation */
            int shifted = temp << 2;    /* Shift operation */
            
            /* Loop-carried dependency on array with distance-1 */
            if (i > 0) {
                /* distance-1 dependency: a[i] depends on a[i-1] */
                a[i] = prev_val + b_val + shifted;
                prev_val = a[i];  /* Update for next iteration */
            } else {
                /* First iteration special case */
                a[i] = b_val + shifted;
                prev_val = a[i];
            }
            
            /* Another use of product to create distance1_uses scenario */
            int extra = (product % 256) | (temp & 0x7F);
            b[i] = (b[i] + extra) ^ 0x55;
            
            /* Additional computation with mixed operations */
            total_sum += (sum & 0xFFFF) - (extra * 3);
        }
        
        /* Volatile update to prevent optimization */
        outer_counter++;
        
        /* Conditional based on random to create control variability */
        if (rand() % 2) {
            total_sum += sum;
        } else {
            total_sum -= sum / 2;
        }
    }
    
    return total_sum;
}

int main() {
    int a[SIZE];
    int b[SIZE];
    int i;
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random data */
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the function with the complex loop */
    long long result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %lld\n", result);
    
    /* Additional print to use arrays */
    printf("First few a values: %d %d %d\n", a[0], a[1], a[2]);
    printf("First few b values: %d %d %d\n", b[0], b[1], b[2]);
    
    return 0;
}
