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
        /* Loop-carried dependency: state from iteration i used in i+1 */
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
        int tmp9 = acc6 ^ tmp8;
        int tmp10 = acc7 * 3;
        int tmp11 = acc8 + 7;
        
        /* Conditional execution based on loop-variant condition */
        if (prev_state & 1) {
            /* Memory access with variable indexing */
            sum += data[(prev_state + i) % size] * 2;
            tmp7 += data[(i * 3) % size];
        } else {
            tmp7 -= data[(i * 5) % size];
        }
        
        /* Update multiple accumulators to keep them live */
        acc1 = tmp1 + tmp7;
        acc2 = tmp2 ^ tmp8;
        acc3 = tmp3 & tmp9;
        acc4 = tmp4 | tmp10;
        acc5 = tmp5 * tmp11;
        acc6 = tmp6 + sum;
        acc7 = tmp7 ^ state;
        acc8 = tmp8 & prev_state;
        
        /* Additional arithmetic to increase instruction mix */
        int tmp12 = (acc1 << 2) | (acc2 >> 3);
        int tmp13 = (acc3 * acc4) + (acc5 % 17);
        int tmp14 = (acc6 & 0xFF) ^ (acc7 | 0x55);
        int tmp15 = (acc8 + tmp12) * (tmp13 - tmp14);
        
        /* More operations to create complex dependency graph */
        if (i & 3) {
            tmp15 += data[(tmp15 + i) % size];
        }
        
        /* Final updates with cross-dependencies */
        acc1 = acc1 + tmp15;
        acc2 = acc2 ^ tmp12;
        acc3 = acc3 * tmp13;
        acc4 = acc4 | tmp14;
    }
    
    /* Combine all results to prevent dead code elimination */
    int result = state + sum + acc1 + acc2 + acc3 + acc4 + 
                 acc5 + acc6 + acc7 + acc8;
    return result;
}

int main() {
    /* Initialize array with pseudo-random values */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Process the loop */
    int result = process_loop(data, ARRAY_SIZE);
    
    /* Print result to ensure computation isn't optimized away */
    printf("Result: %d\n", result);
    
    /* Additional loop with different characteristics */
    volatile int M = 300;
    int arr2[512];
    int total = 0;
    
    for (int i = 0; i < 512; ++i) {
        arr2[i] = rand() % 500;
    }
    
    /* Another loop with different dependency pattern */
    int carry = 1;
    for (int i = 0; i < M; ++i) {
        /* Different loop-carried dependency */
        int old_carry = carry;
        carry = (carry * arr2[i % 512] + i) % 100;
        
        /* More arithmetic operations */
        int x1 = old_carry * i;
        int x2 = arr2[(i + old_carry) % 512];
        int x3 = x1 & x2;
        int x4 = x1 | x2;
        int x5 = x3 ^ x4;
        int x6 = x5 + carry;
        int x7 = x6 * 3;
        int x8 = x7 - x5;
        
        if (i % 2) {
            x8 += arr2[(x8 + i) % 512];
        }
        
        total += x8;
    }
    
    printf("Total: %d\n", total + result);
    
    return 0;
}
