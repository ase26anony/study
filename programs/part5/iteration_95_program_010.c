#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure the loop structure is preserved */
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent unrolling */
    int total_sum = 0;
    
    /* Outer loop with volatile control to prevent optimization */
    while (outer_counter < OUTER_ITERATIONS) {
        int sum = 0;
        int prev_a = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with multiple dependencies and operations */
        for (int i = 0; i < size; i++) {
            /* Multiple loads to create scheduling pressure */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with high latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Another operation with different latency (bitwise) */
            int masked = product & 0xFF;
            
            /* Distance-1 dependency: a[i] depends on a[i-1] */
            int new_a_val;
            if (i > 0) {
                /* True loop-carried dependency with distance 1 */
                new_a_val = prev_a + b_val + (sum & 1);
            } else {
                new_a_val = a_val + 1;
            }
            
            /* Multiple uses of the same value (creates distance1_uses) */
            int use1 = new_a_val * 2;
            int use2 = new_a_val + masked;
            int use3 = new_a_val ^ product;
            
            /* Store with potential aliasing */
            a[i] = new_a_val;
            
            /* Update for next iteration's distance-1 dependency */
            prev_a = new_a_val;
            
            /* More operations to increase scheduling complexity */
            total_sum += (sum & 0x3) + (use1 % 7) - (use2 >> 2) | use3;
        }
        
        /* Volatile increment to prevent outer loop unrolling */
        outer_counter++;
        
        /* Add some control flow variability */
        if (rand() % 2) {
            total_sum += sum;
        }
    }
    
    return total_sum;
}

int main() {
    /* Initialize with different seeds for variability */
    srand(time(NULL));
    
    /* Declare arrays with enough elements for pipelining */
    int a[SIZE];
    int b[SIZE];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the function with the critical loop */
    int result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
