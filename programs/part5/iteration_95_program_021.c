#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent unrolling */
    int sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        /* Complex inner loop with multiple dependencies */
        /* True data dependency: sum = sum + a[i] * b[i] */
        /* Loop-carried dependency: a[i] depends on a[i-1] */
        /* Multiple uses of computed values within iteration */
        
        /* Initialize first element specially to avoid out-of-bounds */
        int prev_a = a[0];
        
        for (i = 0; i < size; i++) {
            /* Load operations with potential cache effects */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Multiple arithmetic operations with different latencies */
            int product = a_val * b_val;      /* Higher latency multiply */
            int shifted = product >> 3;       /* Shift operation */
            int masked = shifted & 0x1F;      /* Bitwise AND */
            
            /* Loop-carried dependency on sum */
            sum = sum + product;              /* Distance-0 dependency */
            
            /* Additional accumulation with different operation */
            sum = sum + masked;               /* Another use of sum */
            
            /* Loop-carried dependency on array a */
            int new_a;
            if (i > 0) {
                /* Distance-1 dependency: a[i] depends on a[i-1] */
                new_a = prev_a + b_val;       /* prev_a is a[i-1] from previous iteration */
            } else {
                new_a = a_val + 1;
            }
            
            /* Multiple uses of new_a within same iteration */
            int doubled = new_a * 2;          /* Use 1 */
            int complemented = ~doubled;      /* Use 2 (bitwise NOT) */
            
            /* Store with potential aliasing */
            a[i] = new_a;
            
            /* Update for next iteration's loop-carried dependency */
            prev_a = new_a;
            
            /* Additional computation with mixed operations */
            int temp = (doubled | complemented) ^ sum;  /* Bitwise OR and XOR */
            sum = sum + (temp & 0xFF);        /* Final accumulation */
        }
        
        outer_counter++;
        
        /* Conditional based on random to create control flow variability */
        if (rand() % 2) {
            /* Extra computation to vary loop body */
            sum = sum ^ 0xABCD;  /* XOR with constant */
        }
    }
    
    return sum;
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
    
    /* Additional print to ensure arrays are used */
    printf("First few a[] values: %d, %d, %d\n", a[0], a[1], a[2]);
    
    return 0;
}
