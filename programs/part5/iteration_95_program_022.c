#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline))
static int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent unrolling */
    int total_sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    for (outer_counter = 0; outer_counter < OUTER_ITERATIONS; outer_counter++) {
        int sum = 0;
        int prev_val = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with multiple dependencies and operations */
        for (i = 0; i < size; i++) {
            /* Multiple loads with potential aliasing */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with non-unit latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Another loop-carried dependency: a[i] depends on a[i-1] */
            int temp = prev_val + b_val;
            
            /* Multiple uses of the same value to create distance1_uses */
            int shifted = temp << 2;      /* Use 1 */
            int masked = shifted & 0xFF;  /* Use 2 */
            int xored = masked ^ product; /* Use 3 */
            
            /* Store with dependency chain */
            a[i] = xored + (i > 0 ? a[i-1] : 0);
            
            /* Update for next iteration's loop-carried dependency */
            prev_val = a[i];
            
            /* Additional operations to create complex scheduling graph */
            int extra_op1 = (sum & 0xFFFF) * (product & 0xFF);
            int extra_op2 = (a_val | b_val) ^ (sum >> 4);
            int extra_op3 = extra_op1 - extra_op2;
            
            /* Volatile to prevent elimination */
            volatile int dummy = extra_op3;
            (void)dummy;
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

/* Another complex loop with different pattern */
__attribute__((noinline))
static int second_modulo_loop(int *a, int *b, int size) {
    int result = 0;
    int i;
    
    /* Different dependency pattern */
    for (i = 1; i < size; i++) {
        /* Strong loop-carried dependency chain */
        int diff = a[i] - a[i-1];          /* distance-1 dependency */
        int scaled = diff * 3;             /* Use of diff */
        int shifted = scaled >> 1;         /* Another use */
        
        /* Cross-iteration through b array */
        int b_combined = b[i] + b[i-1];    /* Another distance-1 */
        int combined = shifted + b_combined;
        
        /* Multiple operations with different latencies */
        int mult_result = combined * a[i];     /* High latency */
        int bit_result = mult_result & 0x7F;   /* Low latency */
        int arith_result = bit_result - diff;  /* Medium latency */
        
        /* Update with loop-carried dependency */
        a[i] = arith_result + result;
        result = a[i];
        
        /* Additional dependency chain */
        b[i] = b[i-1] ^ (result * 2);
    }
    
    return result;
}

int main(void) {
    int a[SIZE];
    int b[SIZE];
    int i, result1, result2;
    
    /* Seed RNG for reproducibility */
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random data */
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the modulo-scheduled loops */
    result1 = modulo_scheduled_loop(a, b, SIZE);
    result2 = second_modulo_loop(a, b, SIZE);
    
    /* Print results to prevent dead code elimination */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    
    return 0;
}
