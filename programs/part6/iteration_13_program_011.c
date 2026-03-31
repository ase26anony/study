/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * with loop-carried dependencies and complex instruction scheduling
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline))
int complex_loop(int *data, int size) {
    volatile int N = 500;          /* Prevent constant propagation */
    volatile int seed = 42;        /* Volatile to prevent optimizations */
    
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
        /* Loop-carried dependency: state from iteration i used in i+1 */
        int prev_state = state;
        
        /* Multiple independent arithmetic operations */
        tmp1 = i * 1103515245;
        tmp2 = i + 12345;
        tmp3 = tmp1 ^ tmp2;
        tmp4 = tmp1 & tmp2;
        tmp5 = tmp1 | tmp2;
        tmp6 = tmp3 * tmp4;
        tmp7 = tmp5 ^ tmp6;
        tmp8 = tmp7 + prev_state;
        tmp9 = tmp8 * 1664525;
        tmp10 = tmp9 + 1013904223;
        
        /* Update state with loop-carried dependency */
        state = (prev_state * 1103515245 + 12345) ^ tmp10;
        
        /* More arithmetic to increase instruction count */
        tmp11 = data[i % size] * 3;
        tmp12 = tmp11 + i;
        tmp13 = tmp12 ^ state;
        tmp14 = tmp13 & 0x7FFFFFFF;
        tmp15 = tmp14 % 1023;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Memory access with variable indexing */
            sum += data[tmp15] * 2;
        } else if (i & 2) {
            /* Alternative path with different operations */
            sum -= data[(i + state) % size] / 3;
        }
        
        /* Update multiple accumulators to keep them live */
        acc1 += tmp1;
        acc2 += tmp2;
        acc3 += tmp3;
        acc4 += tmp4;
        acc5 += tmp5;
        acc6 += tmp6;
        acc7 += tmp7;
        acc8 += tmp8;
        
        /* Additional operations with array access */
        int idx = (i * 13 + state) % size;
        acc1 ^= data[idx];
        acc2 += data[(idx + 7) % size];
        
        /* Complex expression with multiple operators */
        acc3 = (acc3 * 3 + data[i % size]) & 0xFF;
        acc4 = (acc4 ^ (data[(i + 1) % size] * 2)) | 0x1;
        
        /* Nested arithmetic */
        acc5 = ((acc5 << 3) | (acc5 >> 29)) + tmp9;
        acc6 = (acc6 * 5 - tmp10) % 1000;
        
        /* Memory-dependent operation */
        if (data[i % size] > 0) {
            acc7 = acc7 * 2 + 1;
        } else {
            acc7 = acc7 / 2 - 1;
        }
        
        /* Another loop-carried dependency chain */
        acc8 = acc8 + prev_state + tmp15;
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    int result = sum + acc1 + acc2 + acc3 + acc4 + acc5 + acc6 + acc7 + acc8;
    return result;
}

int main() {
    const int SIZE = 1024;
    int *data = (int*)malloc(SIZE * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Execute the complex loop */
    int result = complex_loop(data, SIZE);
    
    /* Use the result to prevent optimization */
    printf("Result: %d\n", result);
    
    free(data);
    return 0;
}
