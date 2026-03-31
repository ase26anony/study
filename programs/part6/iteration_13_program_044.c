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
int compute_loop(int *data, int size) {
    volatile int N = 500;  /* Prevent constant propagation */
    volatile int seed = 42; /* Volatile to prevent optimization */
    
    /* Multiple accumulator variables to create register pressure */
    int state = seed;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state from iteration i used in i+1 */
        int prev_state = state;
        
        /* Multiple independent arithmetic operations */
        int tmp1 = acc1 * i;           /* Multiplication */
        int tmp2 = acc2 + i;           /* Addition */
        int tmp3 = tmp1 ^ tmp2;        /* XOR */
        int tmp4 = acc3 & i;           /* AND */
        int tmp5 = acc4 | i;           /* OR */
        int tmp6 = tmp4 - tmp5;        /* Subtraction */
        int tmp7 = tmp3 << 2;          /* Shift */
        int tmp8 = tmp6 >> 1;          /* Shift */
        
        /* Complex update with loop-carried dependency */
        state = (prev_state * 1103515245 + 12345) ^ data[i % size];
        
        /* Memory access with variable indexing */
        int idx = (state + i) % size;
        int val = data[idx] * 3;
        
        /* Nested control flow */
        if (state & 1) {
            /* Conditional memory access */
            sum += data[(state * i) % size];
            acc1 += val * 2;
        } else {
            acc2 += val / 2;
        }
        
        /* More arithmetic to increase instruction count */
        acc3 = acc3 * 7 + tmp7;
        acc4 = acc4 ^ tmp8;
        acc5 = acc5 + (tmp1 & 0xFF);
        acc6 = acc6 | (tmp2 << 3);
        acc7 = acc7 - (tmp3 % 17);
        acc8 = acc8 ^ (tmp4 * tmp5);
        
        /* Another conditional with different condition */
        if ((i & 3) == 0) {
            int idx2 = (i * 13) % size;
            acc1 += data[idx2];
            acc2 -= data[(idx2 + 1) % size];
        }
        
        /* Use all temporaries to keep them live */
        acc3 += tmp6;
        acc4 ^= tmp7;
        acc5 |= tmp8;
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    int result = state + sum + acc1 + acc2 + acc3 + acc4 + acc5 + acc6 + acc7 + acc8;
    return result;
}

int main() {
    /* Initialize array with pseudo-random values */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Call the computation function */
    int result = compute_loop(data, ARRAY_SIZE);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Additional computation to ensure loop isn't optimized away */
    volatile int check = result;
    if (check > 1000000) {
        printf("Large result detected\n");
    }
    
    return 0;
}
