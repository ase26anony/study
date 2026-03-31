/* modulo-sched-trigger.c
 * Designed to trigger GCC's modulo scheduler debugging output
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-all -fdump-rtl-sched2 -march=native -o modulo_test modulo-sched-trigger.c
 */

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
    int i;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        int sum = 0;
        int prev = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with multiple dependencies */
        for (i = 0; i < size; i++) {
            /* Multiple operations with different latencies */
            int load_a = a[i];
            int load_b = b[i];
            
            /* Integer multiply (higher latency on most architectures) */
            int product = load_a * load_b;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Another loop-carried dependency with distance 1 */
            int temp = prev + load_b;
            
            /* Multiple uses of the same value (creates distance1_uses scenarios) */
            int use1 = temp & 0xFF;      /* Bitwise operation */
            int use2 = use1 ^ product;   /* Another operation using the value */
            
            /* Store with loop-carried dependency */
            a[i] = use2;
            
            /* Update prev for next iteration's loop-carried dependency */
            prev = load_a;
            
            /* Additional operations to create complex scheduling graph */
            int extra_op = (product >> 3) | (use1 << 5);
            b[i] = extra_op + i;  /* Index-dependent operation */
        }
        
        total_sum += sum;
        outer_counter++;
        
        /* Conditional branch based on random to prevent optimization */
        if (rand() % 2) {
            /* Force some control flow variability */
            total_sum += 1;
        }
    }
    
    return total_sum;
}

/* Another complex loop with different patterns */
__attribute__((noinline))
static int second_modulo_loop(int *a, int *b, int size) {
    int result = 0;
    volatile int control = 3;
    
    while (control > 0) {
        int acc = 0;
        int carry = 0;
        
        /* Loop with mixed distance dependencies */
        for (int i = 1; i < size; i++) {
            /* True data dependency (distance 0) */
            int x = a[i] - b[i];
            
            /* Loop-carried dependency (distance 1) */
            int y = a[i-1] + carry;
            
            /* Multiple consumers of y */
            int z1 = y * x;
            int z2 = y & 0x7F;
            
            /* Complex expression with multiple operations */
            int combined = (z1 >> 2) + (z2 << 1);
            
            /* Update with loop-carried dependency */
            a[i] = combined + acc;
            
            /* Accumulate with loop-carried dependency */
            acc = acc + combined;
            
            /* Another distance-1 dependency chain */
            carry = x % 256;
            
            /* Use result in multiple ways */
            result ^= combined;
        }
        
        control--;
        
        /* Random branch to maintain control flow */
        if (rand() % 100 > 50) {
            result += a[0];
        }
    }
    
    return result;
}

int main(void) {
    int a[SIZE];
    int b[SIZE];
    int i, result1, result2;
    
    /* Seed random number generator */
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
    printf("Total: %d\n", result1 + result2);
    
    return 0;
}
