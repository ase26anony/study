#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
unsigned long long modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = OUTER_ITERATIONS;  /* Prevent unrolling */
    unsigned long long total_sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    while (outer_counter-- > 0) {
        unsigned long long sum = 0;
        int prev_val = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with multiple dependencies */
        for (i = 0; i < size; i++) {
            /* Multiple operations with different latencies */
            int a_val = a[i];
            int b_val = b[i];
            
            /* Integer multiply (higher latency) */
            int product = a_val * b_val;
            
            /* Loop-carried dependency on sum */
            sum = sum + product;
            
            /* Loop-carried dependency on array with distance-1 */
            int temp = prev_val + b_val;
            
            /* Multiple uses of the same value (creates distance1_uses) */
            int temp2 = temp & 0xFF;      /* Bitwise operation */
            int temp3 = temp2 | 0x1;      /* Another operation on same value */
            
            /* Store with dependency chain */
            a[i] = temp3;
            prev_val = temp3;  /* Carry to next iteration */
            
            /* Additional arithmetic to create more scheduling pressure */
            sum = sum + (temp2 ^ temp3);  /* Use both computed values */
        }
        
        /* Mix in some conditional control flow in outer loop */
        if (rand() % 2) {
            total_sum += sum;
        } else {
            total_sum += sum >> 1;
        }
    }
    
    return total_sum;
}

/* Another complex loop with different pattern */
__attribute__((noinline))
unsigned long long second_modulo_loop(int *a, int *b, int size) {
    unsigned long long sum1 = 0, sum2 = 0;
    int i;
    
    /* Loop with multiple accumulators and dependencies */
    for (i = 1; i < size; i++) {
        /* Distance-1 dependency on a[i-1] */
        int dep1 = a[i-1] * 3;
        
        /* Multiple uses of dep1 (should trigger distance1_uses) */
        int use1 = dep1 + b[i];
        int use2 = dep1 - b[i-1];
        
        /* Cross-iteration dependency through sum1 */
        sum1 = sum1 + use1 * use2;
        
        /* Another dependency chain */
        int dep2 = (a[i] & 0xF) | (b[i] & 0xF0);
        sum2 = sum2 ^ dep2;  /* Bitwise accumulation */
        
        /* Store with multiple dependencies */
        a[i] = (use1 + use2) * dep2;
    }
    
    return sum1 + sum2;
}

int main() {
    int a[SIZE], b[SIZE];
    int i;
    unsigned long long result1, result2;
    
    srand(time(NULL));
    
    /* Initialize with pseudo-random data */
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the modulo-scheduled loops */
    result1 = modulo_scheduled_loop(a, b, SIZE);
    
    /* Re-initialize for second loop */
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    result2 = second_modulo_loop(a, b, SIZE);
    
    /* Print results to prevent dead code elimination */
    printf("Result 1: %llu\n", result1);
    printf("Result 2: %llu\n", result2);
    printf("Total: %llu\n", result1 + result2);
    
    return 0;
}
