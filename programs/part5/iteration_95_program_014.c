#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure the loop structure remains intact */
__attribute__((noinline))
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent unrolling */
    int total_sum = 0;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        int sum = 0;
        int prev_a = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with complex dependencies */
        for (int i = 0; i < size; i++) {
            /* Multiple loads creating scheduling pressure */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with higher latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Another loop-carried dependency: a[i] depends on a[i-1] */
            int new_a_val;
            if (i > 0) {
                /* distance-1 dependency: uses value from previous iteration */
                new_a_val = prev_a + b_val;
            } else {
                new_a_val = a_val;
            }
            
            /* Multiple uses of the same computed value within iteration */
            /* This creates distance1_uses scenarios */
            int temp1 = new_a_val & 0xFF;      /* Use 1 */
            int temp2 = temp1 | product;       /* Use 2 with product */
            int temp3 = temp2 ^ sum;           /* Use 3 with sum */
            
            /* Store with potential aliasing */
            a[i] = new_a_val + temp3;
            
            /* Update for next iteration's loop-carried dependency */
            prev_a = new_a_val;
            
            /* Additional operations to create complex scheduling graph */
            int extra_op1 = (product >> 3) & 0x1F;
            int extra_op2 = extra_op1 * (i & 0x7);
            total_sum += extra_op2;  /* Accumulate to prevent elimination */
        }
        
        /* Volatile increment to prevent outer loop unrolling */
        outer_counter++;
        
        /* Add inner sum to total */
        total_sum += sum;
        
        /* Conditional based on random to create control flow variability */
        if (rand() % 2) {
            total_sum += 1;  /* Small perturbation */
        }
    }
    
    return total_sum;
}

int main() {
    /* Initialize with different seed each run */
    srand(time(NULL));
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Call the loop function */
    int result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a);
    free(b);
    
    return 0;
}
