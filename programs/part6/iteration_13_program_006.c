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
    volatile int seed = 123;
    
    /* Multiple accumulators to create register pressure */
    int state = seed;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state depends on previous iteration */
        int prev_state = state;
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        int tmp1 = acc1 * i;
        int tmp2 = acc2 + i;
        int tmp3 = tmp1 ^ tmp2;
        int tmp4 = acc3 & i;
        int tmp5 = acc4 | i;
        int tmp6 = tmp3 * tmp4;
        int tmp7 = tmp5 + tmp6;
        int tmp8 = acc5 - i;
        int tmp9 = acc6 * 3;
        int tmp10 = acc7 / 2;
        int tmp11 = tmp8 << 2;
        int tmp12 = tmp9 >> 1;
        
        /* Memory access with variable indexing (creates address calculation) */
        int idx = (i + prev_state) % size;
        sum += data[idx] * 7;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Complex expression with multiple operations */
            acc1 = data[(state + i) % size] + tmp7;
            acc2 = tmp11 ^ tmp12;
        } else {
            acc1 = tmp10 * 5;
            acc2 = tmp9 - tmp8;
        }
        
        /* Update multiple accumulators to keep them live */
        acc3 = tmp1 + tmp2;
        acc4 = tmp3 | tmp4;
        acc5 = tmp5 ^ tmp6;
        acc6 = tmp7 & tmp8;
        acc7 = tmp9 + tmp10;
        acc8 = tmp11 * tmp12;
        
        /* Another loop-carried dependency chain */
        acc8 = acc8 + prev_state;
    }
    
    /* Combine all results to prevent dead code elimination */
    return state + sum + acc1 + acc2 + acc3 + acc4 + acc5 + acc6 + acc7 + acc8;
}

/* Another complex loop with different pattern */
__attribute__((noinline))
int process_loop2(int *data, int size) {
    volatile int M = 300;
    int result = 0;
    int carry = 1;
    
    for (int j = 0; j < M; ++j) {
        /* Different loop-carried dependency pattern */
        int old_carry = carry;
        carry = (carry * 1664525 + 1013904223) & 0x7FFFFFFF;
        
        /* Many temporary variables for register pressure */
        int t1 = data[j % size];
        int t2 = data[(j + old_carry) % size];
        int t3 = t1 * t2;
        int t4 = t1 + t2;
        int t5 = t1 ^ t2;
        int t6 = t3 & t4;
        int t7 = t5 | t6;
        int t8 = t7 << 1;
        int t9 = t8 >> 2;
        int t10 = t9 * 3;
        int t11 = t10 + 7;
        int t12 = t11 - 5;
        int t13 = t12 & 0xFF;
        int t14 = t13 | 0x80;
        int t15 = t14 ^ 0x55;
        
        /* Nested if-else with loop-variant condition */
        if ((j + old_carry) & 3) {
            result += t15 * 2;
        } else {
            result -= t15;
        }
        
        /* Complex addressing mode */
        result += data[(t15 + j) % size];
    }
    
    return result + carry;
}

int main() {
    /* Initialize with random data */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Process multiple loops to increase scheduling opportunities */
    int result1 = process_loop(data, ARRAY_SIZE);
    int result2 = process_loop2(data, ARRAY_SIZE);
    
    /* Use results to prevent optimization */
    printf("Result1: %d, Result2: %d, Combined: %d\n", 
           result1, result2, result1 + result2);
    
    return 0;
}
