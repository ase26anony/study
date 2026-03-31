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
    volatile int seed = 42; /* Volatile to prevent optimizations */
    
    /* Multiple accumulator variables to create register pressure */
    int state = seed;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    
    /* Many temporary variables for high register pressure */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int tmp11, tmp12, tmp13, tmp14, tmp15;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state from iteration i used in i+1 */
        int prev_state = state;
        
        /* Multiple independent arithmetic operations */
        tmp1 = i * 1103515245;
        tmp2 = i + 12345;
        tmp3 = tmp1 ^ tmp2;
        tmp4 = prev_state & 0xFFFF;
        tmp5 = tmp3 | tmp4;
        tmp6 = i * 3;
        tmp7 = i / 7;
        tmp8 = tmp6 + tmp7;
        tmp9 = tmp5 * 2;
        tmp10 = tmp8 - tmp9;
        
        /* Complex update with loop-carried dependency */
        state = (prev_state * 1103515245 + 12345) ^ tmp10;
        
        /* Memory access with variable indexing */
        int idx = (state + i) % size;
        if (idx < 0) idx = -idx;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* True loop-carried dependency for sum */
            sum += data[idx] * (i & 0xFF);
        } else {
            /* Alternative path with different operations */
            sum -= data[(idx * 3) % size] / ((i % 16) + 1);
        }
        
        /* More independent operations to increase instruction count */
        tmp11 = data[(i * 2) % size];
        tmp12 = tmp11 << (i % 8);
        tmp13 = tmp12 ^ state;
        tmp14 = tmp13 * 16807;
        tmp15 = tmp14 % 2147483647;
        
        /* Update multiple accumulators to keep them live */
        acc1 += tmp1;
        acc2 += tmp2;
        acc3 += tmp3;
        acc4 += tmp4;
        acc5 += tmp5;
        acc6 += tmp6;
        acc7 += tmp7;
        acc8 += tmp8;
        
        /* Additional arithmetic to create more dependencies */
        acc1 = (acc1 * 13) ^ tmp9;
        acc2 = (acc2 + tmp10) | tmp11;
        acc3 = acc3 * tmp12;
        acc4 = acc4 ^ tmp13;
        
        /* Nested conditional with loop-variant condition */
        if ((i & 3) == 0) {
            tmp1 = tmp1 * 2;
            tmp2 = tmp2 + state;
        } else if ((i & 3) == 1) {
            tmp3 = tmp3 / 3;
            tmp4 = tmp4 ^ state;
        } else {
            tmp5 = tmp5 << 1;
            tmp6 = tmp6 >> 1;
        }
        
        /* Final cross-iteration dependency */
        state = state ^ (tmp15 & 0xFF);
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    int result = sum + acc1 + acc2 + acc3 + acc4 + acc5 + acc6 + acc7 + acc8;
    result ^= state;
    
    return result;
}

int main() {
    /* Initialize with pseudo-random data */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Process the loop multiple times to ensure execution */
    int total = 0;
    volatile int iterations = 3; /* Prevent unrolling */
    
    for (int j = 0; j < iterations; ++j) {
        total += process_loop(data, ARRAY_SIZE);
        
        /* Modify data slightly between calls */
        for (int i = 0; i < ARRAY_SIZE; i += 37) {
            data[i] = (data[i] * 13 + 17) % 1000;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
