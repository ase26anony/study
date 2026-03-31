/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * with loop-carried dependencies and complex instruction scheduling
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int complex_loop(int* data, int size, volatile int limit) {
    /* Multiple accumulators to create register pressure */
    int state = 123456789;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    
    /* Volatile modifiers to prevent constant propagation */
    volatile int mod1 = 1103515245;
    volatile int mod2 = 12345;
    volatile int mask = 0x7FFFFFFF;
    
    /* Loop with true loop-carried dependency on 'state' */
    for (int i = 0; i < limit; ++i) {
        /* Loop-carried dependency: state from iteration i used in i+1 */
        int prev_state = state;
        
        /* Multiple independent arithmetic operations */
        int tmp1 = prev_state * mod1;
        int tmp2 = tmp1 + mod2;
        state = tmp2 ^ data[i & (ARRAY_SIZE-1)];
        
        /* More parallel arithmetic operations */
        acc1 = acc1 + (i * 3);
        acc2 = acc2 ^ (state >> 1);
        acc3 = acc3 | (tmp1 & 0xFF);
        acc4 = acc4 - (tmp2 % 256);
        
        /* Memory access with variable indexing */
        int idx = (state + i) % ARRAY_SIZE;
        int val = data[idx] * 2;
        
        /* Additional arithmetic chains */
        acc5 = (acc5 * 7) + val;
        acc6 = (acc6 << 3) | (val & 0xF);
        acc7 = acc7 ^ (idx * 11);
        acc8 = acc8 + (data[(i * 13) % ARRAY_SIZE] & 0x3F);
        
        /* Nested control flow */
        if (state & 1) {
            /* Conditional memory access */
            sum += data[state % ARRAY_SIZE];
            acc1 = acc1 ^ (sum & 0xFF);
        } else {
            /* Alternative computation path */
            acc2 = acc2 | (i & 0xF0);
            tmp1 = tmp1 >> 4;
        }
        
        /* More operations to increase instruction count */
        if (i & 2) {
            acc3 = acc3 * 3;
            acc4 = acc4 ^ prev_state;
        }
        
        /* Cross-iteration dependency through multiple variables */
        acc5 = acc6 + acc5;
        acc6 = acc7 ^ acc6;
        acc7 = acc8 | acc7;
        acc8 = acc1 & acc8;
        
        /* Complex expression with multiple operators */
        int complex_expr = ((acc1 * acc2) + (acc3 ^ acc4)) | 
                          ((acc5 - acc6) & (acc7 + acc8));
        sum += complex_expr & 0x3FF;
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    return sum + state + acc1 + acc2 + acc3 + acc4 + 
           acc5 + acc6 + acc7 + acc8;
}

int main() {
    /* Initialize with pseudo-random data */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile loop limit to prevent unrolling */
    volatile int iterations = 500;
    
    /* Call the complex loop multiple times */
    int total = 0;
    for (int run = 0; run < 3; ++run) {
        total += complex_loop(data, ARRAY_SIZE, iterations);
        
        /* Modify data slightly between runs */
        for (int i = 0; i < ARRAY_SIZE; i += 37) {
            data[i] = (data[i] * 13 + 17) % 1000;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
