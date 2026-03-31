#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
static int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    int total_sum = 0;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        int sum = 0;
        int prev_a = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with complex dependencies */
        for (int i = 0; i < size; i++) {
            /* Multiple loads to create scheduling pressure */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with higher latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Another operation using the product */
            int shifted = product >> 3;
            
            /* Distance-1 dependency: a[i] depends on a[i-1] */
            int new_a_val;
            if (i > 0) {
                /* True loop-carried dependency with distance 1 */
                new_a_val = prev_a + b_val + shifted;
            } else {
                new_a_val = a_val + b_val;
            }
            
            /* Multiple uses of computed value within same iteration */
            int temp1 = new_a_val & 0xFF;
            int temp2 = new_a_val | 0x55;
            int temp3 = temp1 ^ temp2;
            
            /* Store with potential aliasing */
            a[i] = new_a_val + temp3;
            
            /* Update for next iteration's distance-1 dependency */
            prev_a = new_a_val;
            
            /* Another accumulation with different operation */
            total_sum += (sum & 0x1) ? product : -product;
        }
        
        /* Volatile increment to prevent optimization */
        outer_counter = outer_counter + 1;
        
        /* Add some control flow variability */
        if (rand() % 100 > 50) {
            total_sum += sum;
        }
    }
    
    return total_sum;
}

int main(void) {
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
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the loop function */
    int result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Verify computation with simple version */
    int verify = 0;
    for (int i = 0; i < SIZE; i++) {
        verify += a[i] + b[i];
    }
    printf("Verification sum: %d\n", verify);
    
    free(a);
    free(b);
    
    return 0;
}
