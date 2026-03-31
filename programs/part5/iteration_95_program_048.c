#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure the loop structure is preserved */
__attribute__((noinline)) 
long modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    long total_sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        long sum = 0;
        volatile int inner_control = rand() % 2;  /* Control flow variability */
        
        /* Critical inner loop with loop-carried dependencies */
        for (i = 0; i < size; i++) {
            /* Multiple loads to create scheduling pressure */
            int a_val = a[i];
            int b_val = b[i];
            volatile int temp;  /* Prevent optimization */
            
            /* Operation with high latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Distance-1 dependency: a[i] depends on a[i-1] */
            if (i > 0) {
                /* Create distance1_uses scenario: multiple uses of a[i-1] */
                int prev_a = a[i-1];
                int delta = prev_a & 0xFF;  /* Bitwise operation */
                a[i] = (a[i] + delta) | (prev_a ^ 0x55);  /* Multiple operations */
                
                /* Another use of the same value to create distance1_uses */
                temp = prev_a * 3;
            } else {
                /* Boundary case */
                a[i] = a[i] ^ 0xAA;
            }
            
            /* More operations to create complex scheduling graph */
            int diff = b_val - a_val;
            sum = sum + (diff & 0x3F);  /* Bitwise AND */
            
            /* Another multiplication with different latency characteristics */
            product = product * (b_val + 1);
            sum = sum + (product >> 2);  /* Shift operation */
            
            /* Store intermediate result with volatile to prevent elimination */
            temp = sum & 0xFFFF;
        }
        
        /* Conditional based on random to create control flow */
        if (inner_control) {
            total_sum += sum;
        } else {
            total_sum += sum * 2;
        }
        
        outer_counter++;
        
        /* Additional operations between outer loop iterations */
        for (i = 0; i < 10; i++) {
            a[i] = (a[i] * 3) & 0xFFF;
        }
    }
    
    return total_sum;
}

int main() {
    int a[SIZE];
    int b[SIZE];
    int i;
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the loop function */
    long result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %ld\n", result);
    
    /* Additional print to ensure array is used */
    printf("Sample a[0]=%d, b[0]=%d\n", a[0], b[0]);
    
    return 0;
}
