#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure the loop structure is preserved */
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    int total_sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        int sum = 0;
        int prev_a = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with complex dependencies */
        for (i = 0; i < size; i++) {
            /* Multiple loads with potential aliasing */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Operation with high latency (multiplication) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Another loop-carried dependency: a[i] depends on a[i-1] */
            int temp = prev_a + b_val;
            
            /* Multiple uses of the same value to create distance1_uses */
            int shifted = temp >> 3;
            int masked = shifted & 0xFF;
            int xored = masked ^ product;
            
            /* Update array with loop-carried dependency */
            a[i] = xored + (i > 0 ? a[i-1] : 0);
            
            /* Store previous value for next iteration */
            prev_a = a[i];
            
            /* Additional operations with different latencies */
            int bit_op = (sum & 0xFFFF) | (product & 0xFFFF0000);
            total_sum += bit_op & 0xFF;
        }
        
        /* Volatile increment to prevent optimization */
        outer_counter++;
        
        /* Conditional based on random to create control flow variability */
        if (rand() % 2) {
            total_sum += sum;
        } else {
            total_sum -= sum;
        }
    }
    
    return total_sum;
}

/* Another complex loop with different patterns */
__attribute__((noinline))
int secondary_loop(int *a, int *b, int size) {
    int result = 0;
    volatile int control = 3;
    
    while (control-- > 0) {
        int acc = 0;
        int carry = 1;
        
        for (int i = 1; i < size; i++) {
            /* Strong loop-carried dependency chain */
            int diff = a[i] - a[i-1];
            int prod = diff * b[i];
            
            /* Multiple dependent operations */
            acc = acc + prod;
            int shifted = acc << 2;
            int masked = shifted & 0x3FF;
            
            /* Update with distance-1 dependency */
            a[i] = (a[i-1] + masked) & 0x7FFF;
            
            /* Another dependency chain */
            carry = (carry * 3 + b[i]) & 0xFF;
            result += carry;
        }
        
        /* Mix operations */
        result ^= acc;
    }
    
    return result;
}

int main() {
    /* Initialize with different seeds for variability */
    srand(time(NULL));
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the loop function multiple times */
    int total = 0;
    
    /* Multiple calls to increase scheduling opportunities */
    for (int iter = 0; iter < 3; iter++) {
        total += modulo_scheduled_loop(a, b, SIZE);
        
        /* Modify data between calls */
        for (int i = 0; i < SIZE; i++) {
            b[i] = (b[i] + 1) & 0x7FF;
        }
    }
    
    /* Add secondary loop result */
    total += secondary_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Cleanup */
    free(a);
    free(b);
    
    return 0;
}
