#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure the loop structure remains intact */
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    int total_sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        int sum = 0;
        int prev_val = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with complex dependencies */
        for (i = 0; i < size; i++) {
            /* Multiple loads with potential aliasing */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operations with different latencies */
            int product = a_val * b_val;      /* Multi-cycle multiply */
            int shifted = product << 2;       /* Fast shift */
            int masked = shifted & 0xFFF;     /* Bitwise operation */
            
            /* Loop-carried dependency on sum (distance-1) */
            sum = sum + masked;
            
            /* Another loop-carried dependency on array (distance-1) */
            int new_val;
            if (i > 0) {
                /* True loop-carried dependency: a[i] depends on a[i-1] */
                new_val = prev_val + b_val;
            } else {
                new_val = a_val;
            }
            
            /* Multiple uses of the same value within iteration */
            int temp1 = new_val * 3;
            int temp2 = new_val / 2;
            int combined = temp1 - temp2;     /* Mixed operations */
            
            a[i] = combined;
            prev_val = new_val;  /* Carry to next iteration */
            
            /* Additional arithmetic to create more scheduling pressure */
            total_sum += (sum & 0x1);  /* Use LSB of sum */
        }
        
        /* Volatile increment to prevent optimization */
        outer_counter++;
        
        /* Conditional based on random to create control variability */
        if (rand() % 2) {
            total_sum += sum;
        }
    }
    
    return total_sum;
}

int main() {
    int a[SIZE];
    int b[SIZE];
    int i, result;
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the loop function */
    result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional print to use arrays */
    printf("First element: %d\n", a[0]);
    
    return 0;
}
