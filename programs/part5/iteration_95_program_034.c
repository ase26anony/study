#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITER 4

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = OUTER_ITER;  /* Prevent unrolling */
    int total_sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    while (outer_counter-- > 0) {
        int sum = 0;
        int prev_a = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with complex dependencies */
        for (i = 0; i < size; i++) {
            /* Multiple loads with potential aliasing */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with high latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Another loop-carried dependency: a[i] depends on a[i-1] */
            int new_a;
            if (i > 0) {
                /* distance-1 dependency: uses value from previous iteration */
                new_a = prev_a + b_val;
            } else {
                new_a = a_val;
            }
            
            /* Multiple uses of the same value (creates distance1_uses scenarios) */
            int temp1 = new_a & 0xFF;      /* Bitwise operation */
            int temp2 = new_a | 0x100;     /* Another use of new_a */
            int temp3 = temp1 ^ temp2;     /* Combination of both */
            
            /* Store with potential anti-dependencies */
            a[i] = new_a + temp3;
            
            /* Update for next iteration's loop-carried dependency */
            prev_a = new_a;
            
            /* Additional arithmetic to create more scheduling opportunities */
            total_sum += (sum & 0x3F) - (product >> 2);
        }
        
        /* Conditional branch based on random to create control flow variability */
        if (rand() % 2) {
            total_sum += sum;
        }
    }
    
    return total_sum;
}

int main() {
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
    
    /* Call the function with the complex loop */
    result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Also print a few array values to ensure stores aren't optimized away */
    printf("Sample a[0]=%d, a[100]=%d, a[500]=%d\n", a[0], a[100], a[500]);
    
    return 0;
}
