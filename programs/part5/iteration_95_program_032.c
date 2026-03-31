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
        int prev_a = a[0];  /* For loop-carried dependency */
        
        /* Critical inner loop with multiple dependencies */
        for (i = 0; i < size; i++) {
            /* Multiple loads with potential aliasing */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with non-unit latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Another loop-carried dependency: a[i] depends on a[i-1] */
            int temp = prev_a + b_val;
            
            /* Multiple uses of the same value to create distance1_uses */
            int temp2 = temp & 0xFF;      /* Use 1: bitwise AND */
            int temp3 = temp2 | 0x80;     /* Use 2: bitwise OR */
            int temp4 = temp3 ^ 0x55;     /* Use 3: bitwise XOR */
            
            /* Store with potential anti-dependency */
            a[i] = temp4;
            
            /* Update for next iteration's loop-carried dependency */
            prev_a = temp4;
            
            /* Additional arithmetic to increase scheduling complexity */
            total_sum += (sum & 1) ? product : -product;
        }
        
        /* Volatile increment to prevent optimization */
        outer_counter++;
        
        /* Conditional branch based on random to create control variability */
        if (rand() % 2) {
            total_sum += sum;
        } else {
            total_sum -= sum;
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
    
    /* Additional print to ensure arrays are used */
    printf("First element of a: %d\n", a[0]);
    printf("First element of b: %d\n", b[0]);
    
    return 0;
}
