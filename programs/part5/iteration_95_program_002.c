#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure separate function analysis */
__attribute__((noinline))
static int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent optimization */
    int total_sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    for (outer_counter = 0; outer_counter < OUTER_ITERATIONS; outer_counter++) {
        int sum = 0;
        int prev_a = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with complex dependencies */
        for (i = 0; i < size; i++) {
            /* Multiple loads with potential aliasing */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with non-unit latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Another loop-carried dependency: a[i] depends on a[i-1] */
            int new_a_val;
            if (i > 0) {
                /* Create distance-1 dependency */
                new_a_val = prev_a + b_val;
                /* Multiple uses of the same value to trigger distance1_uses */
                int temp1 = new_a_val & 0xFF;      /* Bitwise operation */
                int temp2 = new_a_val | 0x55;      /* Another operation */
                new_a_val = temp1 ^ temp2;         /* Third use */
            } else {
                new_a_val = b_val;
            }
            
            /* Store with potential anti-dependency */
            a[i] = new_a_val;
            prev_a = new_a_val;  /* For next iteration */
            
            /* Additional operations to create scheduling pressure */
            int extra_op1 = sum * 3;      /* Another multiplication */
            int extra_op2 = extra_op1 - product;  /* Subtraction */
            b[i] = extra_op2 & 0xFFFF;    /* Bitwise and store */
        }
        
        /* Mix in some conditional control flow in outer loop */
        if (rand() % 2) {
            total_sum += sum;
        } else {
            total_sum -= sum;
        }
    }
    
    return total_sum;
}

int main(void) {
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
    
    /* Also print some array values to ensure stores aren't optimized away */
    printf("Sample a[0]=%d, a[100]=%d, b[0]=%d\n", a[0], a[100], b[0]);
    
    return 0;
}
