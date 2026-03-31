#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure the loop structure is preserved */
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    int total_sum = 0;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        int sum = 0;
        int prev_a = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with multiple dependencies */
        for (int i = 0; i < size; i++) {
            /* Load operations with potential cache effects */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Multiple operations with different latencies */
            int product = a_val * b_val;      /* Higher latency multiply */
            int shifted = product >> 3;       /* Bitwise operation */
            int masked = shifted & 0x0F;      /* Another bitwise op */
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + masked + (product & 1);
            
            /* Another loop-carried dependency: a[i] depends on a[i-1] */
            int new_a_val;
            if (i > 0) {
                /* Distance-1 dependency: uses value from previous iteration */
                new_a_val = prev_a + b_val + (sum & 0x7);
            } else {
                new_a_val = a_val + b_val;
            }
            
            /* Multiple uses of the same computed value (creates distance1_uses) */
            int temp = new_a_val * 3;
            a[i] = temp + (temp >> 2);  /* Two uses of 'temp' */
            
            /* Update for next iteration's loop-carried dependency */
            prev_a = new_a_val;
            
            /* Additional arithmetic to create more scheduling opportunities */
            sum = sum ^ (b_val & 0xFF);  /* Bitwise XOR */
            sum = sum - (a_val % 16);    /* Modulo operation */
        }
        
        total_sum += sum;
        outer_counter++;
        
        /* Conditional based on random to prevent optimization */
        if (rand() % 2) {
            total_sum ^= 1;  /* Minor perturbation */
        }
    }
    
    return total_sum;
}

int main() {
    /* Initialize with different seeds for variability */
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
        a[i] = rand() % 256;
        b[i] = rand() % 256;
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
