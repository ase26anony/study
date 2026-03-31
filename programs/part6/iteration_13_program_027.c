/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms -fno-unroll-loops -fno-tree-vectorize modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline))
int compute_loop(int *data, int size) {
    volatile int N = 500;  /* Prevent constant propagation */
    volatile int seed = 42;
    
    /* Multiple accumulators to create register pressure */
    int state = seed;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    int acc9 = 0, acc10 = 0, acc11 = 0, acc12 = 0;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state from iteration i used in i+1 */
        int prev_state = state;
        
        /* Multiple independent arithmetic operations */
        int tmp1 = acc1 * i + 12345;
        int tmp2 = acc2 & (i << 3);
        int tmp3 = tmp1 | tmp2;
        int tmp4 = acc3 ^ (i * 1103515245);
        int tmp5 = acc4 + (tmp3 * 1664525);
        int tmp6 = acc5 - (tmp4 % 997);
        int tmp7 = acc6 * ((tmp5 & 0xFF) + 1);
        int tmp8 = acc7 | (tmp6 ^ 0x5555);
        int tmp9 = acc8 + (tmp7 * 3);
        int tmp10 = acc9 & (tmp8 | 0xAAAA);
        
        /* Memory access with variable indexing (creates address computation) */
        int idx = (prev_state + i) % size;
        int val = data[idx] * 636413622;
        
        /* Conditional execution based on loop-variant condition */
        if (i & 1) {  /* Simple condition that changes per iteration */
            /* Complex operation with multiple dependencies */
            state = (prev_state * 1103515245 + 12345) ^ val;
            sum += data[state % size];
        } else {
            /* Alternative path with different operations */
            state = (prev_state * 1664525 + 1013904223) | val;
            sum -= data[(state * 3) % size];
        }
        
        /* Update multiple accumulators to keep them live */
        acc1 = tmp1 + state;
        acc2 = tmp2 ^ acc1;
        acc3 = tmp3 | acc2;
        acc4 = tmp4 - acc3;
        acc5 = tmp5 * acc4;
        acc6 = tmp6 & acc5;
        acc7 = tmp7 + acc6;
        acc8 = tmp8 ^ acc7;
        acc9 = tmp9 | acc8;
        acc10 = tmp10 - acc9;
        acc11 = acc11 + acc10 * 7;
        acc12 = acc12 ^ (acc11 << 1);
        
        /* Additional memory access with complex addressing */
        int idx2 = (i * 13 + state) % size;
        if (idx2 > size / 2) {
            acc1 += data[idx2];
        } else {
            acc2 -= data[(idx2 * 2) % size];
        }
        
        /* More arithmetic to increase instruction count */
        int tmp11 = (acc1 * acc2) / (i + 1);
        int tmp12 = (acc3 & acc4) | (acc5 ^ acc6);
        int tmp13 = tmp11 + tmp12 * 3;
        int tmp14 = (acc7 << 2) + (acc8 >> 3);
        
        /* Final updates with cross-iteration dependencies */
        acc1 = (acc1 + tmp13) % 10007;
        acc2 = (acc2 ^ tmp14) + 1;
        acc3 = acc3 * 3 + tmp13;
        acc4 = acc4 | (tmp14 & 0xFF);
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    int result = state + sum + acc1 + acc2 + acc3 + acc4 + acc5 + acc6 + 
                 acc7 + acc8 + acc9 + acc10 + acc11 + acc12;
    
    return result;
}

int main() {
    /* Initialize array with pseudo-random values */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Call the computation loop */
    int result = compute_loop(data, ARRAY_SIZE);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Additional computation to ensure loop isn't optimized away */
    volatile int check = result;
    if (check & 1) {
        printf("Odd result\n");
    } else {
        printf("Even result\n");
    }
    
    return 0;
}
