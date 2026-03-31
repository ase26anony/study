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
        int prev_a = a[0];  /* Initialize for loop-carried dependency */
        
        /* Inner loop with multiple dependencies */
        for (int i = 0; i < size; i++) {
            /* Multiple loads to create scheduling pressure */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with higher latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Another loop-carried dependency: a[i] depends on a[i-1] */
            int temp = prev_a + b_val;
            
            /* Multiple uses of 'temp' to create distance1_uses scenarios */
            int masked = temp & 0xFF;      /* First use */
            int shifted = masked << 2;     /* Second use */
            int combined = shifted | 0x1;  /* Third use */
            
            /* Store with loop-carried dependency */
            a[i] = combined;
            prev_a = combined;  /* Update for next iteration */
            
            /* Additional arithmetic to create more scheduling nodes */
            int extra_op = (product ^ b_val) + (a_val & 0x3F);
            sum += extra_op & 0x1;  /* Modulo 2 addition */
        }
        
        total_sum += sum;
        outer_counter++;
        
        /* Conditional to prevent optimization */
        if (rand() % 100 > 50) {
            total_sum ^= 0xABCD;  /* Random modification */
        }
    }
    
    return total_sum;
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
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
    printf("Sample a[0]: %d, a[100]: %d, a[500]: %d\n", 
           a[0], a[100], a[500]);
    
    return 0;
}
