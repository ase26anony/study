#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure separate function analysis */
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    int total_sum = 0;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        int sum = 0;
        int prev_val = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with complex dependencies */
        for (int i = 0; i < size; i++) {
            /* Multiple loads with potential aliasing */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with high latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency through sum */
            sum = sum + product;
            
            /* Distance-1 dependency: use previous iteration's value */
            int temp = prev_val + b_val;
            
            /* Multiple uses of the same computed value */
            int masked1 = temp & 0xFF;
            int masked2 = temp & 0xF0;
            int combined = masked1 | masked2;
            
            /* Update with loop-carried dependency */
            a[i] = combined + (i > 0 ? a[i-1] : 0);
            
            /* Store previous value for next iteration */
            prev_val = a[i];
            
            /* Additional operations to create scheduling pressure */
            int extra_op1 = product ^ b_val;
            int extra_op2 = extra_op1 * (i & 3);
            sum += extra_op2;
        }
        
        total_sum += sum;
        outer_counter++;
        
        /* Conditional to prevent predictable control flow */
        if (rand() % 100 > 50) {
            total_sum += 1;  /* Small perturbation */
        }
    }
    
    return total_sum;
}

int main() {
    /* Initialize with random data */
    srand(time(NULL));
    
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the loop function */
    int result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Verify with simple computation */
    int verify = 0;
    for (int i = 0; i < SIZE; i++) {
        verify += a[i] + b[i];
    }
    printf("Verification sum: %d\n", verify);
    
    free(a);
    free(b);
    
    return 0;
}
