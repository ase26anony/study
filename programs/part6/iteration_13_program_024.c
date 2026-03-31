/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * with loop-carried dependencies and complex instruction scheduling
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int complex_loop(int* data, int size, volatile int limit) {
    /* Multiple accumulators to create register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    int state = 123456789;
    volatile int modifier = 7;  /* Prevent constant propagation */
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < limit; ++i) {
        /* Loop-carried dependency: state depends on previous iteration */
        int prev_state = state;
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        int tmp1 = acc1 * i;
        int tmp2 = acc2 + (i ^ modifier);
        int tmp3 = tmp1 | tmp2;
        int tmp4 = acc3 & (i << 2);
        int tmp5 = acc4 ^ (i * 3);
        int tmp6 = tmp3 + tmp4;
        int tmp7 = tmp5 * tmp6;
        int tmp8 = acc5 + (tmp7 >> 4);
        
        /* Memory access with variable indexing */
        int idx = (i + prev_state) % size;
        int val = data[idx] * modifier;
        
        /* Conditional execution path */
        if (state & 1) {
            acc1 += val;
            acc2 ^= data[(idx + 1) % size];
        } else {
            acc3 |= val;
            acc4 &= data[(idx + 2) % size];
        }
        
        /* More arithmetic to increase instruction count */
        acc5 = acc5 * 3 + i;
        acc6 = (acc6 << 1) ^ prev_state;
        acc7 = acc7 + (val % 256);
        acc8 = acc8 - (tmp8 & 0xFF);
        
        /* Additional operations to create more dependencies */
        int tmp9 = acc1 * acc2;
        int tmp10 = acc3 | acc4;
        int tmp11 = tmp9 ^ tmp10;
        int tmp12 = acc5 + acc6;
        int tmp13 = acc7 * acc8;
        int tmp14 = tmp11 & tmp12;
        int tmp15 = tmp13 | tmp14;
        
        /* Use results to prevent dead code elimination */
        acc1 = (acc1 + tmp15) & 0x7FFFFFFF;
        acc2 = (acc2 ^ tmp15) | 1;
    }
    
    /* Combine all accumulators */
    return acc1 + acc2 + acc3 + acc4 + acc5 + acc6 + acc7 + acc8 + state;
}

int main() {
    const int SIZE = 1024;
    int* data = (int*)malloc(SIZE * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile loop limit to prevent optimization */
    volatile int N = 500;
    
    /* Call the complex loop function */
    int result = complex_loop(data, SIZE, N);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    free(data);
    return 0;
}
