#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure the loop structure is preserved */
__attribute__((noinline))
static int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent optimization */
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
            
            /* Operations with different latencies */
            int product = a_val * b_val;      /* Higher latency multiply */
            int temp = product & 0xFF;        /* Bitwise operation */
            
            /* Loop-carried dependency: sum accumulates across iterations */
            sum = sum + temp;                 /* Distance-1 dependency */
            
            /* Another loop-carried dependency on array 'a' */
            int new_val;
            if (i > 0) {
                /* Use previous iteration's value */
                new_val = prev_val + b_val;   /* Another distance-1 dependency */
            } else {
                new_val = a_val;
            }
            
            /* Multiple uses of the same computed value */
            int shifted = new_val << 2;       /* Use new_val twice */
            int masked = shifted & new_val;   /* Second use creates distance1_uses scenario */
            
            /* Store with potential anti-dependency */
            a[i] = masked;
            prev_val = new_val;               /* Update for next iteration */
            
            /* Additional arithmetic to create more scheduling pressure */
            total_sum += (sum ^ product);     /* Mix of operations */
        }
        
        /* Volatile counter update prevents loop unrolling */
        outer_counter++;
        
        /* Add some conditional control flow in outer loop */
        if (rand() % 2) {
            total_sum += sum;
        }
    }
    
    return total_sum;
}

int main(void) {
    /* Initialize with random data */
    srand(time(NULL));
    
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    
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
