#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    int total_sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        int sum = 0;
        volatile int inner_control = outer_counter;  /* Prevent optimization */
        
        /* Critical inner loop with multiple dependencies */
        for (i = 0; i < size; i++) {
            /* Load operations with potential cache effects */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Multiple operations with different latencies */
            int product = a_val * b_val;      /* Higher latency multiply */
            int shifted = product >> 3;       /* Shift operation */
            int masked = shifted & 0xFF;      /* Bitwise operation */
            
            /* Loop-carried dependency: sum accumulates across iterations */
            sum = sum + masked;               /* Distance-1 dependency */
            
            /* Another loop-carried dependency with distance-1 */
            if (i > 0) {
                /* Use previous iteration's value */
                int prev_val = a[i-1];
                int combined = prev_val + b_val;  /* Distance-1 use */
                a[i] = combined & 0x7FFF;         /* Update with bitwise */
                
                /* Multiple uses of the same value within iteration */
                int temp = combined * 2;          /* Another use */
                sum = sum - (temp >> 1);          /* Complex dependency chain */
            } else {
                /* Boundary case */
                a[i] = (b_val * 3) & 0x7FFF;
            }
            
            /* Additional operations to create scheduling pressure */
            int extra_op = (a_val ^ b_val) | (product & 0xFFFF);
            sum = sum + (extra_op % 256);         /* Modulo operation */
            
            /* Create register pressure with multiple variables */
            int tmp1 = a_val + i;
            int tmp2 = b_val - i;
            int tmp3 = tmp1 * tmp2;
            sum = sum + (tmp3 & 0xFF);
        }
        
        total_sum += sum;
        outer_counter++;
        
        /* Conditional branch based on random to prevent optimization */
        if (rand() % 2) {
            total_sum = total_sum ^ 1;  /* Minor perturbation */
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
    
    /* Additional print to ensure array values are used */
    printf("Sample values: a[0]=%d, b[0]=%d\n", a[0], b[0]);
    
    return 0;
}
