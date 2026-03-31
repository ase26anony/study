#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
static int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent unrolling */
    int total_sum = 0;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        int sum = 0;
        int prev_a = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with multiple dependencies */
        for (int i = 0; i < size; i++) {
            /* Multiple loads creating scheduling pressure */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with higher latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Another loop-carried dependency with distance 1 */
            int temp = prev_a + b_val;
            
            /* Multiple uses of the same value to create distance1_uses */
            int masked = temp & 0xFF;
            int shifted = temp >> 8;
            int combined = masked | shifted;
            
            /* Update array with loop-carried dependency */
            if (i > 0) {
                a[i] = combined + a[i-1];  /* True distance-1 dependency */
            } else {
                a[i] = combined;
            }
            
            /* Store previous value for next iteration */
            prev_a = a_val;
            
            /* Additional operations with different latencies */
            int diff = a_val - b_val;
            int xor_result = a_val ^ b_val;
            int and_result = a_val & b_val;
            
            /* Use results to prevent elimination */
            sum += diff + xor_result - and_result;
        }
        
        total_sum += sum;
        outer_counter++;
        
        /* Conditional branch based on random to create control variability */
        if (rand() % 2) {
            total_sum += 1;  /* Small perturbation */
        }
    }
    
    return total_sum;
}

int main(void) {
    /* Initialize with random data */
    srand(time(NULL));
    
    int a[SIZE];
    int b[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the loop function */
    int result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional print to use arrays */
    printf("First element: a[0]=%d, b[0]=%d\n", a[0], b[0]);
    
    return 0;
}
