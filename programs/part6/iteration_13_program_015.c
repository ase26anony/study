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
int process_loop(int *data, volatile int limit) {
    /* Loop-carried state with true dependency (distance=1) */
    unsigned int state = 0x12345678;
    
    /* Multiple accumulators to create register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    int acc6 = 0, acc7 = 0, acc8 = 0, acc9 = 0, acc10 = 0;
    
    /* Many temporary variables for high register pressure */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int tmp11, tmp12, tmp13, tmp14, tmp15;
    
    /* Volatile modifiers to prevent constant propagation */
    volatile int mod1 = 1103515245;
    volatile int mod2 = 12345;
    volatile int mod3 = 67890;
    
    for (int i = 0; i < limit; ++i) {
        /* 1. Loop-carried dependency: state from iteration i used in i+1 */
        state = (state * mod1 + mod2) ^ data[i & (ARRAY_SIZE-1)];
        
        /* 2. Multiple independent arithmetic operations */
        tmp1 = acc1 * i;           /* Multiplication */
        tmp2 = acc2 + i;           /* Addition */
        tmp3 = tmp1 ^ tmp2;        /* XOR */
        tmp4 = acc3 & mod3;        /* AND */
        tmp5 = acc4 | state;       /* OR */
        tmp6 = tmp1 - tmp2;        /* Subtraction */
        tmp7 = tmp3 << 2;          /* Shift */
        tmp8 = tmp4 >> 1;          /* Shift */
        tmp9 = tmp5 * tmp6;        /* Another multiplication */
        tmp10 = tmp7 + tmp8;       /* Another addition */
        
        /* 3. Memory access with variable indexing (creates address calc) */
        int idx = (state + i) & (ARRAY_SIZE-1);
        tmp11 = data[idx] * 7;
        tmp12 = data[(idx + 1) & (ARRAY_SIZE-1)] + 3;
        
        /* 4. Conditional execution based on loop-variant condition */
        if (state & 1) {           /* Branch inside loop */
            tmp13 = tmp11 * tmp12;
            acc5 += tmp13;
        } else {
            tmp13 = tmp11 + tmp12;
            acc6 += tmp13;
        }
        
        /* 5. More operations to increase instruction count */
        tmp14 = (tmp9 & 0xFF) | (tmp10 << 8);
        tmp15 = tmp14 ^ (i * 17);
        
        /* 6. Update multiple accumulators (keep values live) */
        acc1 = tmp1 + 1;
        acc2 = tmp2 - 1;
        acc3 = tmp3 ^ acc3;
        acc4 = tmp4 | acc4;
        acc7 = tmp5 + acc7;
        acc8 = tmp6 - acc8;
        acc9 = tmp14 * acc9;
        acc10 = tmp15 + acc10;
        
        /* 7. Another loop-carried dependency chain */
        acc1 = acc1 ^ acc2;
        acc2 = acc2 + acc1;
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    int result = state + acc1 + acc2 + acc3 + acc4 + acc5 + 
                 acc6 + acc7 + acc8 + acc9 + acc10;
    
    return result;
}

int main() {
    /* Initialize with pseudo-random data */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile loop limit to prevent constant propagation/unrolling */
    volatile int iterations = 500;
    
    /* Call the processing function multiple times */
    int total = 0;
    for (int run = 0; run < 3; ++run) {
        total += process_loop(data, iterations);
    }
    
    printf("Result: %d\n", total);
    
    return 0;
}
