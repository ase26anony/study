#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
long modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent optimization */
    long total_sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        long sum = 0;
        int prev_val = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with complex dependencies */
        for (i = 0; i < size; i++) {
            /* Multiple loads creating scheduling pressure */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with higher latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency through sum */
            sum = sum + product;
            
            /* Additional operations with different latencies */
            int temp1 = product & 0xFF;      /* Bitwise AND */
            int temp2 = temp1 ^ b_val;       /* XOR operation */
            int temp3 = temp2 - a_val;       /* Subtraction */
            
            /* Create distance-1 dependency: use value from previous iteration */
            if (i > 0) {
                /* This creates distance1_uses scenario */
                a[i] = prev_val + temp3;     /* Update based on previous iteration */
            }
            
            /* Multiple uses of the same value within iteration */
            int use1 = temp3 * 2;
            int use2 = use1 | 0x1;           /* OR operation */
            prev_val = use2;                 /* Store for next iteration */
            
            /* Another computation to increase register pressure */
            total_sum += (sum & 0x3);        /* Use sum in different way */
        }
        
        /* Volatile update to prevent loop unrolling */
        outer_counter++;
        
        /* Conditional branch based on random to create control variability */
        if (rand() % 2) {
            total_sum += sum;
        }
    }
    
    return total_sum;
}

int main() {
    int a[SIZE];
    int b[SIZE];
    int i;
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Call the loop function */
    long result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %ld\n", result);
    
    return 0;
}
