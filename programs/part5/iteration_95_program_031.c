#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 5

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
static int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent optimization */
    int total_sum = 0;
    int i, j;
    
    /* Outer loop with volatile control */
    for (j = 0; j < OUTER_ITERATIONS; j++) {
        outer_counter = j;  /* Volatile write to prevent optimization */
        
        int sum = 0;
        int prev_val = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with complex dependencies */
        for (i = 0; i < size; i++) {
            /* Multiple loads to create register pressure */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with high latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Another operation with different latency */
            int shifted = product >> 3;
            
            /* Distance-1 dependency: a[i] depends on a[i-1] */
            int new_val;
            if (i > 0) {
                /* True loop-carried dependency with distance 1 */
                new_val = prev_val + b_val + (sum & 0xFF);
            } else {
                new_val = b_val;
            }
            
            /* Multiple uses of the same value (creates distance1_uses) */
            int temp1 = new_val * 7;
            int temp2 = new_val / 3;
            int combined = (temp1 ^ temp2) & 0xFFFF;
            
            /* Store with potential aliasing */
            a[i] = combined + shifted;
            
            /* Update for next iteration's distance-1 dependency */
            prev_val = new_val;
            
            /* Another computation using sum (creates more dependencies) */
            total_sum += (sum & 0x7F) * (i & 0x3F);
        }
        
        /* Conditional based on random to create control flow variability */
        if (rand() % 2) {
            total_sum += sum;
        }
    }
    
    return total_sum;
}

int main(void) {
    int a[SIZE];
    int b[SIZE];
    int i, result;
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random data */
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the loop function */
    result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Also print a few array values to ensure stores aren't optimized away */
    printf("Sample a[0]=%d, a[100]=%d, a[1000]=%d\n", 
           a[0], a[100], a[1000]);
    
    return 0;
}
