#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITER 4

/* Prevent inlining to ensure the loop structure remains intact */
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    int total_sum = 0;
    
    /* Outer loop with volatile control to prevent optimization */
    while (outer_counter < OUTER_ITER) {
        int sum = 0;
        int prev_a = a[0];  /* Initialize for loop-carried dependency */
        
        /* Inner loop with complex dependencies for modulo scheduling */
        for (int i = 0; i < size; i++) {
            /* Multiple loads with potential aliasing */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with high latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum accumulates across iterations */
            sum = sum + product;
            
            /* Another loop-carried dependency: a[i] depends on a[i-1] */
            int temp = prev_a + b_val;
            
            /* Bitwise operations with different latencies */
            temp = temp ^ (product & 0xFF);
            
            /* Conditional operation to create control variability */
            if (temp > 0) {
                a[i] = temp;
            } else {
                a[i] = temp + 1;
            }
            
            /* Update for next iteration's loop-carried dependency */
            prev_a = a[i];
            
            /* Additional arithmetic to create more scheduling pressure */
            sum = sum - (b_val & 0x3F);
            sum = sum + (a_val | 0x40);
            
            /* Create distance-1 use scenario */
            int intermediate = sum * 2;
            sum = intermediate / 2;  /* Uses result from same iteration */
        }
        
        total_sum += sum;
        outer_counter++;
        
        /* Add some control flow variability */
        if (rand() % 2) {
            total_sum = total_sum ^ 0xABCD;
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
    
    /* Verify computation with a simple check */
    int verify = 0;
    for (int i = 0; i < SIZE; i++) {
        verify += a[i] + b[i];
    }
    printf("Verification sum: %d\n", verify);
    
    free(a);
    free(b);
    
    return 0;
}
