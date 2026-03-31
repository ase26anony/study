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
    
    /* Multiple accumulator variables to create register pressure */
    int state = seed;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    int acc6 = 0, acc7 = 0, acc8 = 0, acc9 = 0, acc10 = 0;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state depends on previous iteration */
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        int tmp1 = acc1 * i;
        int tmp2 = acc2 + i;
        int tmp3 = tmp1 ^ tmp2;
        int tmp4 = acc3 & i;
        int tmp5 = acc4 | i;
        int tmp6 = acc5 - i;
        int tmp7 = acc6 * 3;
        int tmp8 = acc7 + 7;
        int tmp9 = acc8 << 2;
        int tmp10 = acc9 >> 1;
        
        /* Complex expression with multiple operations */
        int complex1 = (tmp1 * tmp2) + (tmp3 & tmp4);
        int complex2 = (tmp5 | tmp6) ^ (tmp7 - tmp8);
        int complex3 = (tmp9 << 1) | (tmp10 >> 1);
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Memory access with variable indexing */
            sum += data[(state + i) % size] * 2;
            acc1 += data[(i * 3) % size];
        } else {
            acc2 += data[(i * 5) % size];
        }
        
        /* Additional conditional with different condition */
        if (i & 3) {
            acc3 += complex1;
            acc4 += complex2;
        } else {
            acc5 += complex3;
            acc6 += state;
        }
        
        /* More arithmetic to increase instruction count */
        acc7 = acc7 * 13 + i;
        acc8 = acc8 ^ (state >> 4);
        acc9 = acc9 + (i % 16);
        acc10 = acc10 - (state % 8);
        
        /* Another loop-carried dependency chain */
        acc1 = acc1 + acc2;
        acc2 = acc2 ^ acc3;
        acc3 = acc3 * 17 + 1;
        
        /* Memory store with variable index */
        if ((i & 7) == 0) {
            data[(i * 11) % size] = state;
        }
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    return sum + state + acc1 + acc2 + acc3 + acc4 + acc5 + 
           acc6 + acc7 + acc8 + acc9 + acc10;
}

/* Another loop function with different pattern */
__attribute__((noinline))
int process_loop2(int *data, int size) {
    volatile int M = 300;
    int result = 0;
    int carry = 1;
    
    for (int j = 0; j < M; ++j) {
        /* Different loop-carried dependency pattern */
        carry = carry * 1664525 + 1013904223;
        
        /* Many temporary variables for register pressure */
        int t1 = data[j % size];
        int t2 = data[(j + 1) % size];
        int t3 = data[(j + 2) % size];
        int t4 = data[(j + 3) % size];
        int t5 = data[(j + 4) % size];
        int t6 = data[(j + 5) % size];
        int t7 = data[(j + 6) % size];
        int t8 = data[(j + 7) % size];
        
        /* Complex dependency chain */
        int v1 = t1 + t2;
        int v2 = t3 * t4;
        int v3 = t5 & t6;
        int v4 = t7 | t8;
        int v5 = v1 ^ v2;
        int v6 = v3 + v4;
        int v7 = v5 * v6;
        int v8 = v7 - carry;
        
        /* Conditional update */
        if (v8 > 0) {
            result += v8;
        } else {
            result -= (-v8) & 0xFF;
        }
        
        /* Update carry for next iteration */
        carry = (carry + v8) & 0x7FFFFFFF;
    }
    
    return result;
}

int main() {
    /* Initialize with pseudo-random data */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Process multiple loops to increase scheduling opportunities */
    int result1 = process_loop(data, ARRAY_SIZE);
    int result2 = process_loop2(data, ARRAY_SIZE);
    
    /* Use results to prevent optimization */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Combined: %d\n", result1 + result2);
    
    return 0;
}
