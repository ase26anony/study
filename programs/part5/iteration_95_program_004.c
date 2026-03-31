#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = OUTER_ITERATIONS;  /* Prevent unrolling */
    int total_sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    while (outer_counter-- > 0) {
        int sum = 0;
        int prev_val = a[0];  /* For loop-carried dependency */
        
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
            int temp = prev_val + b_val;
            
            /* Multiple uses of the same value (creates distance1_uses) */
            int masked1 = temp & 0xFF;
            int masked2 = temp & 0xFF00;
            int combined = masked1 | masked2;
            
            /* Store with potential anti-dependency */
            a[i] = combined;
            
            /* Update for next iteration's loop-carried dependency */
            prev_val = combined;
            
            /* Additional operations to create scheduling pressure */
            int extra_op1 = product ^ b_val;
            int extra_op2 = extra_op1 * 3;
            sum += extra_op2 & 1;  /* Modulo operation alternative */
        }
        
        /* Mix results with outer loop */
        total_sum += sum;
        
        /* Conditional based on random to create control variability */
        if (rand() % 2) {
            total_sum ^= 1;  /* Small perturbation */
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
    
    /* Call the loop function */
    result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Also print a few array values to ensure stores aren't optimized away */
    printf("Sample a[0]: %d, a[%d]: %d\n", a[0], SIZE-1, a[SIZE-1]);
    
    return 0;
}
