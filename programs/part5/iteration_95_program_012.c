#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure loop structure remains intact */
__attribute__((noinline))
static int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    int total_sum = 0;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        int sum = 0;
        int prev_a = a[0];  /* Initialize for loop-carried dependency */
        
        /* Inner loop with multiple dependencies */
        for (int i = 0; i < size; i++) {
            /* Multiple loads with potential aliasing */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with non-unit latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Another loop-carried dependency with distance 1 */
            int temp = prev_a + b_val;
            
            /* Multiple uses of the same value to create distance1_uses */
            int masked1 = temp & 0xFF;
            int masked2 = temp & 0xF0;
            int combined = masked1 | masked2;
            
            /* Update array with loop-carried dependency */
            a[i] = combined + (i > 0 ? a[i-1] : 0);
            
            /* Store previous value for next iteration */
            prev_a = a_val;
            
            /* Additional operations to create complex scheduling graph */
            int diff = a_val - b_val;
            int shifted = diff << 2;
            int xored = shifted ^ product;
            
            /* Use results to prevent elimination */
            sum += xored & 1;
        }
        
        total_sum += sum;
        outer_counter++;
        
        /* Conditional branch based on random to prevent optimization */
        if (rand() % 2) {
            total_sum += 1;
        }
    }
    
    return total_sum;
}

int main(void) {
    /* Initialize with random data */
    int a[SIZE];
    int b[SIZE];
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the loop function */
    int result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Also print a few array values to ensure stores aren't optimized away */
    printf("Sample a[0]=%d, a[100]=%d, b[0]=%d\n", a[0], a[100], b[0]);
    
    return 0;
}
