/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * with loop-carried dependencies and complex instruction mix
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int process_loop(int *data, int size) {
    volatile int N = 500;           /* Prevent constant propagation */
    volatile int seed = 42;         /* Volatile to prevent optimization */
    
    /* Multiple accumulators to create register pressure */
    int state = seed;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    
    /* Many temporary variables for high register pressure */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int tmp11, tmp12, tmp13, tmp14, tmp15;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state depends on previous iteration */
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        tmp1 = acc1 * i;           /* Multiplication */
        tmp2 = acc2 + i;           /* Addition */
        tmp3 = tmp1 ^ tmp2;        /* XOR */
        tmp4 = acc3 & i;           /* AND */
        tmp5 = acc4 | i;           /* OR */
        tmp6 = tmp3 << 2;          /* Shift */
        tmp7 = tmp4 >> 1;          /* Shift */
        tmp8 = tmp5 * 3;           /* Multiplication with constant */
        tmp9 = tmp6 + tmp7;        /* Addition chain */
        tmp10 = tmp8 - tmp9;       /* Subtraction */
        
        /* More operations to increase instruction count */
        tmp11 = tmp10 * state;
        tmp12 = tmp11 ^ 0x5555;
        tmp13 = tmp12 & 0xAAAA;
        tmp14 = tmp13 | 0x3333;
        tmp15 = tmp14 + acc5;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Memory access with variable indexing */
            sum += data[(state + i) % size];
            
            /* More arithmetic in conditional path */
            acc6 = acc6 * 2 + data[(i * 3) % size];
        } else {
            /* Alternative path with different operations */
            acc7 = acc7 ^ data[(i * 5) % size];
        }
        
        /* Update multiple accumulators (keeps variables live) */
        acc1 = tmp1 + 1;
        acc2 = tmp2 - 1;
        acc3 = tmp3 ^ acc3;
        acc4 = tmp4 | acc4;
        acc5 = tmp15;
        acc8 = (acc8 * 7 + i) % 100;
        
        /* Another loop-carried dependency chain */
        if (i > 0) {
            /* Use value from previous iteration */
            acc1 = acc1 + tmp10;  /* tmp10 computed in current iteration */
        }
        
        /* Complex expression with multiple operators */
        sum += ((acc1 * acc2) + (acc3 & acc4) - (acc5 ^ acc6)) / (acc7 + 1);
    }
    
    /* Combine all results to prevent dead code elimination */
    int result = state + sum + acc1 + acc2 + acc3 + acc4 + 
                 acc5 + acc6 + acc7 + acc8;
    
    return result;
}

int main() {
    /* Initialize with pseudo-random data */
    int data[ARRAY_SIZE];
    
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Process the loop */
    int result = process_loop(data, ARRAY_SIZE);
    
    /* Print result to ensure computation isn't optimized away */
    printf("Result: %d\n", result);
    
    return 0;
}
