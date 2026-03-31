#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 5

/* Prevent inlining to ensure the loop structure remains intact */
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    int total_sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        int sum = 0;
        int prev_a = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with multiple dependencies */
        for (i = 0; i < size; i++) {
            /* Load operations with potential cache effects */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Multiple operations with different latencies */
            int product = a_val * b_val;      /* Higher latency multiply */
            int shifted = product << 2;       /* Shift operation */
            int masked = shifted & 0xFFF;     /* Bitwise AND */
            
            /* Loop-carried dependency on sum */
            sum = sum + masked;               /* Distance-1 dependency */
            
            /* Another loop-carried dependency on array a */
            int new_a_val;
            if (i > 0) {
                /* True loop-carried dependency: a[i] depends on a[i-1] */
                new_a_val = prev_a + b_val;   /* Distance-1 dependency */
            } else {
                new_a_val = a_val;
            }
            
            /* Multiple uses of the same value within iteration */
            int temp = new_a_val * 3;         /* Another multiply */
            int temp2 = temp + (sum & 0xF);   /* Use sum again */
            a[i] = temp2;                     /* Store back */
            
            prev_a = new_a_val;               /* Update for next iteration */
            
            /* Additional operations to create complex scheduling graph */
            int extra_op1 = (b_val ^ a_val) & 0x7F;
            int extra_op2 = (product | extra_op1) + 1;
            total_sum += extra_op2 & 0x3;     /* Accumulate to prevent elimination */
        }
        
        /* Volatile increment to prevent optimization */
        outer_counter++;
        
        /* Add some control flow variability */
        if (rand() % 100 > 50) {
            total_sum += sum;
        } else {
            total_sum -= (sum & 0xFF);
        }
    }
    
    return total_sum;
}

/* Another complex loop with different pattern */
__attribute__((noinline))
int secondary_modulo_loop(int *a, int *b, int size) {
    int sum1 = 0, sum2 = 0;
    int i;
    
    /* Loop with multiple accumulators and dependencies */
    for (i = 1; i < size; i++) {
        /* Cross-iteration dependency chain */
        int diff = a[i] - a[i-1];             /* Distance-1 dependency */
        int prod = diff * b[i];               /* Multiply with distance-1 input */
        
        /* Two separate accumulation chains */
        sum1 = sum1 + prod;                   /* Distance-1 dependency */
        sum2 = sum2 + (prod >> 1);            /* Another distance-1 dependency */
        
        /* Update array with mixed operations */
        int mixed = (a[i] & b[i]) | (sum1 & 0xFF);
        a[i] = mixed + (sum2 % 256);
        
        /* Additional operations to increase register pressure */
        int extra1 = (b[i] * 7) + (a[i-1] & 0xF);
        int extra2 = (extra1 ^ sum1) + i;
        sum1 += extra2 & 0x3;
    }
    
    return sum1 + sum2;
}

int main() {
    int a[SIZE], b[SIZE];
    int i, result1, result2;
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random data */
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the modulo-scheduled loop */
    result1 = modulo_scheduled_loop(a, b, SIZE);
    
    /* Re-initialize for second loop */
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    result2 = secondary_modulo_loop(a, b, SIZE);
    
    /* Print results to prevent dead code elimination */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Total: %d\n", result1 + result2);
    
    return 0;
}
