/* modulo-sched-test.c
 * Test program to trigger GCC's modulo scheduling dependency analysis
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fno-unroll-loops -fno-tree-vectorize -fdump-rtl-sms modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed separately */
__attribute__((noinline))
static int complex_loop(int *data, int size, volatile int limit) {
    /* Multiple accumulators to create register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    int state = 123456789;
    int prev_state = 0;  /* For loop-carried dependency */
    
    /* Many temporary variables for register pressure */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int tmp11, tmp12, tmp13, tmp14, tmp15;
    
    /* Volatile modifier to prevent constant propagation */
    volatile int mod1 = 7, mod2 = 13, mod3 = 31;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < limit; ++i) {
        /* Loop-carried dependency: state depends on previous iteration */
        prev_state = state;
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        tmp1 = i * mod1;
        tmp2 = i + mod2;
        tmp3 = tmp1 ^ tmp2;
        tmp4 = tmp1 & tmp2;
        tmp5 = tmp1 | tmp2;
        tmp6 = tmp3 * tmp4;
        tmp7 = tmp5 + tmp6;
        tmp8 = tmp7 ^ prev_state;  /* Uses value from previous iteration */
        tmp9 = tmp8 * 3;
        tmp10 = tmp9 / 2;
        tmp11 = tmp10 << 2;
        tmp12 = tmp11 >> 1;
        tmp13 = tmp12 & 0xFF;
        tmp14 = tmp13 | 0x80;
        tmp15 = tmp14 ^ 0x55;
        
        /* Memory access with variable indexing */
        int idx = (i + state) % size;
        int idx2 = (i * 2 + prev_state) % size;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            acc1 += data[idx] * mod1;
            acc2 += data[idx2] ^ tmp15;
        } else {
            acc3 += data[idx] | tmp14;
            acc4 += data[idx2] & tmp13;
        }
        
        /* More arithmetic mixing previous values */
        acc5 = acc5 * 3 + tmp1;
        acc6 = acc6 ^ tmp2 + i;
        acc7 = acc7 + (tmp3 * tmp4) / 7;
        acc8 = (acc8 << 1) | (state & 1);
        
        /* Additional operations to create more dependencies */
        tmp1 = tmp1 + acc1;
        tmp2 = tmp2 * acc2;
        tmp3 = tmp3 ^ acc3;
        tmp4 = tmp4 | acc4;
        
        /* Use all temporaries to keep them live */
        acc1 = acc1 + (tmp5 ^ tmp6);
        acc2 = acc2 * (tmp7 & tmp8);
        acc3 = acc3 | (tmp9 ^ tmp10);
        acc4 = acc4 & (tmp11 | tmp12);
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    return acc1 + acc2 + acc3 + acc4 + acc5 + acc6 + acc7 + acc8 + state;
}

int main(void) {
    /* Initialize with pseudo-random data */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile loop limit to prevent constant propagation */
    volatile int iterations = 500;
    
    /* Call the complex loop multiple times */
    int total_result = 0;
    for (int run = 0; run < 3; ++run) {
        total_result += complex_loop(data, ARRAY_SIZE, iterations);
    }
    
    printf("Result: %d\n", total_result);
    return 0;
}
