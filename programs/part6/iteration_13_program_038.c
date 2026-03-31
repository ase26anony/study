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
int complex_loop(int *data, int size) {
    volatile int N = 500;  /* Prevent constant propagation */
    int i;
    
    /* Loop-carried state variables */
    int state = 123456789;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    
    /* Many temporary variables to create register pressure */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int tmp11, tmp12, tmp13, tmp14, tmp15;
    
    /* Volatile modifiers to prevent optimization */
    volatile int mod1 = 1103515245;
    volatile int mod2 = 12345;
    volatile int mod3 = 7;
    volatile int mod4 = 31;
    
    for (i = 0; i < N; ++i) {
        /* 1. Loop-carried dependency: state depends on previous iteration */
        state = (state * mod1 + mod2) ^ data[i % size];
        
        /* 2. Multiple independent arithmetic operations */
        tmp1 = i * mod3;
        tmp2 = state & mod4;
        tmp3 = tmp1 + tmp2;
        tmp4 = tmp1 ^ tmp2;
        tmp5 = tmp3 * tmp4;
        tmp6 = tmp2 - tmp1;
        tmp7 = tmp5 | tmp6;
        tmp8 = tmp7 << 2;
        tmp9 = tmp8 >> 1;
        tmp10 = tmp9 & 0xFF;
        
        /* More operations to increase instruction count */
        tmp11 = data[(i + tmp10) % size];
        tmp12 = tmp11 * i;
        tmp13 = tmp12 + state;
        tmp14 = tmp13 ^ tmp10;
        tmp15 = tmp14 % 256;
        
        /* 3. Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Memory access with variable indexing */
            sum += data[(state + i) % size];
            acc1 += tmp15;
        } else {
            acc2 += tmp15 * 2;
        }
        
        /* 4. Multiple accumulators with different update patterns */
        acc3 += tmp3;
        acc4 ^= tmp4;
        acc5 = (acc5 * 3) + tmp5;
        
        /* Additional operations to create more dependencies */
        if (i & 3) {  /* Another conditional */
            tmp1 = data[(i * 2) % size];
            tmp2 = data[(i * 3) % size];
            acc1 += tmp1 - tmp2;
        }
        
        /* Cross-iteration dependency through array */
        if (i > 0) {
            data[i % size] = (data[i % size] + data[(i-1) % size]) & 0xFFFF;
        }
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    return sum + acc1 + acc2 + acc3 + acc4 + acc5 + state;
}

/* Another complex loop with different pattern */
__attribute__((noinline))
int nested_loop(int *data, int size) {
    volatile int outer_limit = 100;
    volatile int inner_limit = 10;
    int i, j;
    int result = 0;
    
    for (i = 0; i < outer_limit; ++i) {
        int inner_state = i * 100;
        
        for (j = 0; j < inner_limit; ++j) {
            /* Loop-carried dependency in inner loop */
            inner_state = (inner_state * 1664525 + 1013904223) & 0x7FFFFFFF;
            
            /* Multiple operations */
            int idx = (inner_state + j) % size;
            int val = data[idx];
            
            /* Complex expression with many temporaries */
            int t1 = val * i;
            int t2 = val + j;
            int t3 = t1 ^ t2;
            int t4 = t3 << (j & 3);
            int t5 = t4 >> 1;
            int t6 = t5 + inner_state;
            int t7 = t6 * 3;
            int t8 = t7 / 2;
            int t9 = t8 | 0xFF;
            int t10 = t9 & 0x3F;
            
            /* Conditional update */
            if ((i + j) & 1) {
                result += t10;
            } else {
                result -= t10;
            }
            
            /* Update array element */
            data[idx] = (data[idx] + t10) & 0xFF;
        }
    }
    
    return result;
}

int main() {
    int i;
    int *data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Execute complex loops */
    int result1 = complex_loop(data, ARRAY_SIZE);
    int result2 = nested_loop(data, ARRAY_SIZE);
    
    /* Use results to prevent optimization */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Combined: %d\n", result1 + result2);
    
    /* Additional computation to use array */
    int checksum = 0;
    for (i = 0; i < ARRAY_SIZE; ++i) {
        checksum ^= data[i];
    }
    printf("Checksum: %d\n", checksum);
    
    free(data);
    return 0;
}
