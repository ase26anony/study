#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITER 4

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    int total_sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITER) {
        int sum = 0;
        int prev_val = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with complex dependencies */
        for (i = 0; i < size; i++) {
            /* Multiple loads with potential aliasing */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with non-unit latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Another loop-carried dependency with distance 1 */
            int temp = prev_val + b_val;
            
            /* Multiple uses of the same value to create distance1_uses */
            int use1 = temp & 0xFF;      /* First use - bitwise operation */
            int use2 = use1 * product;   /* Second use - multiplication */
            int use3 = use2 ^ sum;       /* Third use - XOR */
            
            /* Update array with loop-carried dependency */
            if (i > 0) {
                /* True distance-1 dependency: a[i] depends on a[i-1] */
                a[i] = (a[i-1] + use3) & 0xFFFF;
            } else {
                a[i] = use3 & 0xFFFF;
            }
            
            /* Update prev_val for next iteration */
            prev_val = temp;
            
            /* Additional operations to create scheduling pressure */
            int extra_op1 = (sum >> 3) * 7;
            int extra_op2 = (product << 2) | 0x1F;
            b[i] = (b[i] + extra_op1 - extra_op2) & 0x7FFF;
        }
        
        total_sum += sum;
        outer_counter++;  /* Volatile prevents optimization */
        
        /* Conditional to create control flow variability */
        if (rand() % 100 > 50) {
            total_sum ^= 0xABCD;  /* Modify sum based on random condition */
        }
    }
    
    return total_sum;
}

/* Another complex loop with different pattern */
__attribute__((noinline))
int secondary_loop(int *a, int *b, int size) {
    int sum = 0;
    int i;
    
    /* Loop with mixed operations and dependencies */
    for (i = 1; i < size; i++) {
        /* Chain of dependencies */
        int val1 = a[i] * 3;
        int val2 = b[i] + val1;
        int val3 = a[i-1] * val2;  /* Distance-1 dependency */
        int val4 = val3 - b[i-1];   /* Another distance-1 */
        
        /* Multiple uses of val4 */
        a[i] = (val4 >> 1) + (val4 & 1);
        b[i] = b[i-1] ^ val4;      /* Distance-1 dependency */
        
        sum += val4;
        
        /* Additional pressure operations */
        sum = (sum * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return sum;
}

int main() {
    int a[SIZE], b[SIZE];
    int i, result1, result2;
    
    srand(time(NULL));
    
    /* Initialize with pseudo-random data */
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the loop function multiple times */
    result1 = modulo_scheduled_loop(a, b, SIZE);
    
    /* Re-initialize for second loop */
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    result2 = secondary_loop(a, b, SIZE);
    
    /* Print results to prevent dead code elimination */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Combined: %d\n", result1 ^ result2);
    
    return 0;
}
