/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fno-unroll-loops -fno-tree-vectorize -fdump-rtl-sms modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline))
int process_loop(int *data, int size) {
    volatile int N = 500;  /* Prevent constant propagation */
    int i;
    
    /* Multiple accumulator variables to create register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    int acc6 = 0, acc7 = 0, acc8 = 0, acc9 = 0, acc10 = 0;
    
    /* Loop-carried state with distance=1 dependency */
    int state = 0x12345678;
    
    /* Volatile modifiers to prevent optimization */
    volatile int mod1 = 3, mod2 = 7, mod3 = 11;
    
    for (i = 0; i < N; i++) {
        /* Loop-carried dependency: state from iteration i used in i+1 */
        int prev_state = state;
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        int tmp1 = acc1 * mod1;
        int tmp2 = acc2 + mod2;
        int tmp3 = tmp1 ^ tmp2;
        int tmp4 = acc3 & mod3;
        int tmp5 = acc4 | (i << 2);
        int tmp6 = acc5 - (i % 16);
        int tmp7 = acc6 * (i + 1);
        int tmp8 = acc7 + (prev_state & 0xFF);
        int tmp9 = acc8 ^ (tmp3 << 1);
        int tmp10 = acc9 | (tmp4 >> 2);
        
        /* Complex memory access with variable indexing */
        int idx = (i + prev_state) % size;
        if (idx < 0) idx = -idx;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* True loop-carried dependency chain */
            acc1 = acc1 + data[idx] * mod1;
            acc2 = acc2 ^ data[(idx + 1) % size];
        } else {
            /* Alternative dependency chain */
            acc3 = acc3 | data[(idx * 2) % size];
            acc4 = acc4 - data[(idx + mod2) % size];
        }
        
        /* More arithmetic to increase instruction count */
        acc5 = (acc5 * 3) + (tmp5 & 0xFFFF);
        acc6 = (acc6 >> 1) | (tmp6 << 15);
        acc7 = acc7 + tmp7 - (i % 8);
        acc8 = acc8 ^ (tmp8 * 2);
        acc9 = (acc9 + tmp9) | 0x1;
        acc10 = acc10 + (tmp10 * prev_state);
        
        /* Additional conditional with nested logic */
        if ((i & 3) == 0) {
            int idx2 = (state + i) % size;
            acc1 = acc1 ^ data[idx2];
            acc10 = acc10 + data[(idx2 + 5) % size];
        }
        
        /* Cross-iteration dependency with distance 1 */
        if (i > 0) {
            acc2 = acc2 + (prev_state % 256);
        }
        
        /* More operations to ensure complex scheduling graph */
        int tmp11 = (acc1 * i) + (acc2 >> 2);
        int tmp12 = (acc3 & i) | (acc4 << 1);
        int tmp13 = (acc5 ^ i) + (acc6 & 0xFF);
        int tmp14 = (acc7 * mod1) - (acc8 % 64);
        int tmp15 = (acc9 | 0xAA) ^ (acc10 & 0x55);
        
        /* Update accumulators with new values */
        acc1 = (acc1 + tmp11) & 0x7FFFFFFF;
        acc2 = (acc2 ^ tmp12) | 1;
        acc3 = (acc3 - tmp13) & 0xFFFF;
        acc4 = (acc4 | tmp14) ^ 0x1234;
        acc5 = (acc5 + tmp15) % 10007;
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    int result = acc1 + acc2 + acc3 + acc4 + acc5 + 
                 acc6 + acc7 + acc8 + acc9 + acc10;
    return result ^ state;
}

int main() {
    int i;
    int data[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Process the loop multiple times */
    int total = 0;
    volatile int iterations = 3;
    
    for (i = 0; i < iterations; i++) {
        total += process_loop(data, ARRAY_SIZE);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
