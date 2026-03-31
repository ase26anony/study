/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * with loop-carried dependencies and complex scheduling patterns
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int process_loop(int *data, int size, int seed) {
    volatile int N = 500;          /* Prevent constant propagation */
    volatile int mod1 = 7;         /* Volatile modifiers */
    volatile int mod2 = 13;
    
    int state = seed;
    int sum = 0;
    
    /* Multiple accumulator variables for register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    int acc6 = 0, acc7 = 0, acc8 = 0, acc9 = 0, acc10 = 0;
    
    /* Many temporary variables to increase register pressure */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int tmp11, tmp12, tmp13, tmp14, tmp15;
    
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state depends on previous iteration */
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        tmp1 = acc1 * i;           /* Multiplication */
        tmp2 = acc2 + i;           /* Addition */
        tmp3 = tmp1 ^ tmp2;        /* XOR */
        tmp4 = acc3 & (i * mod1);  /* Bitwise AND */
        tmp5 = acc4 | (i + mod2);  /* Bitwise OR */
        tmp6 = tmp3 - tmp4;        /* Subtraction */
        tmp7 = tmp5 << 2;          /* Shift */
        tmp8 = tmp6 >> 1;
        tmp9 = tmp7 * tmp8;
        tmp10 = tmp9 % 256;
        
        /* More operations to create scheduling nodes */
        tmp11 = (acc5 * 3) / 2;
        tmp12 = (acc6 + 1) * 2;
        tmp13 = tmp11 & tmp12;
        tmp14 = tmp13 | tmp10;
        tmp15 = tmp14 ^ state;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Memory access with variable indexing */
            sum += data[(state + i) % size];
            acc1 += data[(i * 3) % size];
        } else {
            acc2 += data[(i * 5) % size];
        }
        
        /* Additional conditional with different condition */
        if ((i & 3) == 0) {
            acc3 = acc3 * 2 + 1;
            tmp1 = data[(i + acc3) % size] * 3;
        }
        
        /* Update multiple accumulators to keep them live */
        acc4 = acc4 + tmp15;
        acc5 = acc5 ^ tmp14;
        acc6 = acc6 | tmp13;
        acc7 = acc7 & tmp12;
        acc8 = acc8 - tmp11;
        acc9 = acc9 + tmp10;
        acc10 = acc10 * 2 + tmp9;
        
        /* More arithmetic to create parallel operations */
        tmp1 = (acc1 * acc2) + (acc3 / 2);
        tmp2 = (acc4 & acc5) | (acc6 ^ acc7);
        tmp3 = tmp1 * tmp2;
        tmp4 = tmp3 % 100;
        
        /* Use results to prevent dead code elimination */
        if (tmp4 > 50) {
            acc1 = acc1 + tmp4;
        } else {
            acc2 = acc2 - tmp4;
        }
        
        /* Another loop-carried dependency chain */
        acc3 = (acc3 * 17 + acc4) % 1000;
        
        /* Complex expression with multiple operators */
        acc5 = ((acc5 << 3) | (acc6 >> 2)) & 0xFF;
    }
    
    /* Combine all accumulators to ensure computations are used */
    int result = sum + acc1 + acc2 + acc3 + acc4 + acc5 + 
                 acc6 + acc7 + acc8 + acc9 + acc10;
    
    return result;
}

int main() {
    /* Initialize with pseudo-random data */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Process multiple times to ensure loop execution */
    int total = 0;
    volatile int iterations = 3;
    
    for (int j = 0; j < iterations; ++j) {
        int seed = rand() % 100;
        int result = process_loop(data, ARRAY_SIZE, seed);
        total += result;
        printf("Iteration %d: result = %d\n", j, result);
    }
    
    printf("Total: %d\n", total);
    
    return 0;
}
