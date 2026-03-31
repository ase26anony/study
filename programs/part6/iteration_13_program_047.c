/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling pass and exercise the
 * dependence edge calculation logic in modulo-sched.cc lines 596-606
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inline function to ensure the loop is processed independently */
__attribute__((noinline)) 
int compute_loop(int *data, int size, volatile int limit) {
    /* Loop-carried state variables */
    int state = 0x12345678;
    int sum = 0;
    int acc1 = 1, acc2 = 2, acc3 = 3, acc4 = 4, acc5 = 5;
    
    /* Many temporary variables to create register pressure */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int tmp11, tmp12, tmp13, tmp14, tmp15;
    
    /* Volatile modifiers to prevent constant propagation */
    volatile int mod1 = 1103515245;
    volatile int mod2 = 12345;
    volatile int mask = 0x7FFFFFFF;
    
    /* Main loop with true loop-carried dependencies */
    for (int i = 0; i < limit; ++i) {
        /* 1. Loop-carried dependency: state depends on previous iteration */
        state = (state * mod1 + mod2) ^ data[i % size];
        
        /* 2. Multiple independent arithmetic operations */
        tmp1 = acc1 * i;           /* Multiplication */
        tmp2 = acc2 + i;           /* Addition */
        tmp3 = tmp1 ^ tmp2;        /* XOR */
        tmp4 = acc3 & i;           /* AND */
        tmp5 = acc4 | i;           /* OR */
        tmp6 = tmp1 - tmp2;        /* Subtraction */
        tmp7 = tmp3 << 2;          /* Shift */
        tmp8 = tmp4 >> 1;          /* Shift */
        tmp9 = tmp5 * 3;           /* Multiplication with constant */
        tmp10 = tmp6 + tmp7;       /* Chained operation */
        
        /* More operations to increase instruction count */
        tmp11 = tmp8 ^ tmp9;
        tmp12 = tmp10 & 0xFF;
        tmp13 = tmp11 | 0xAA;
        tmp14 = tmp12 * tmp13;
        tmp15 = tmp14 % 256;
        
        /* 3. Memory access with variable indexing */
        int idx = (i + state) % size;
        sum += data[idx] * tmp15;
        
        /* 4. Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Complex address calculation */
            int cond_idx = (state ^ i) % size;
            acc1 += data[cond_idx] * 2;
            acc2 ^= data[(cond_idx + 1) % size];
        } else {
            acc3 *= (data[i % size] & 0xF) + 1;
        }
        
        /* Additional conditional with different condition */
        if ((i & 3) == 0) {
            acc4 += tmp15;
            acc5 ^= state;
        }
        
        /* Update accumulators to keep them live across iterations */
        acc1 = (acc1 * 3 + 1) & mask;
        acc2 = (acc2 ^ tmp1) + i;
        acc3 = (acc3 - tmp2) | 1;
        acc4 = (acc4 + tmp3) ^ 0x55;
        acc5 = (acc5 * 5) % 1000;
        
        /* Create anti-dependencies by reusing temporaries */
        tmp1 = acc1 + acc2;
        tmp2 = acc3 ^ acc4;
        tmp3 = tmp1 * tmp2;
        
        /* Another memory access pattern */
        sum += data[(i * 7) % size] * acc5;
    }
    
    /* Combine all results to prevent dead code elimination */
    return sum + acc1 + acc2 + acc3 + acc4 + acc5 + state;
}

int main() {
    const int SIZE = 1024;
    int *data = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile loop limit to prevent constant propagation */
    volatile int N = 500;
    
    /* Call the computation loop */
    int result = compute_loop(data, SIZE, N);
    
    printf("Result: %d\n", result);
    
    free(data);
    return 0;
}
