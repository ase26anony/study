#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure the loop structure remains intact */
__attribute__((noinline))
static int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = OUTER_ITERATIONS;  /* Prevent unrolling */
    int total_sum = 0;
    
    /* Outer loop with volatile control to prevent optimization */
    while (outer_counter-- > 0) {
        int sum = 0;
        int prev_val = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with multiple dependencies and operations */
        for (int i = 0; i < size; i++) {
            /* Multiple loads with potential aliasing */
            volatile int load_a = a[i];  /* Force load operation */
            volatile int load_b = b[i];  /* Force load operation */
            
            /* Operation with non-unit latency (multiplication) */
            int product = load_a * load_b;
            
            /* Loop-carried dependency: sum accumulates across iterations */
            sum = sum + product;
            
            /* Another loop-carried dependency: a[i] depends on a[i-1] */
            int temp = prev_val + load_b;
            
            /* Multiple uses of the same value to create distance1_uses */
            int use1 = temp & 0xFF;      /* Bitwise operation */
            int use2 = use1 ^ product;   /* Another operation on same value */
            
            /* Store with potential anti-dependency */
            a[i] = use2;
            prev_val = use2;  /* Update for next iteration */
            
            /* Additional arithmetic to create more scheduling opportunities */
            int extra_op = (product >> 3) | (use1 << 5);
            b[i] = extra_op + i;  /* Index-dependent store */
        }
        
        /* Mix in some conditional control flow in outer loop */
        if (rand() % 2) {
            total_sum += sum;
        } else {
            total_sum -= sum / 2;
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
    printf("Sample values: a[0]=%d, a[100]=%d, b[0]=%d, b[100]=%d\n", 
           a[0], a[100], b[0], b[100]);
    
    return 0;
}
