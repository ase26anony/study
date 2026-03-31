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
int complex_loop(int* data, int size, volatile int limit) {
    /* Loop-carried state with true dependency (distance = 1) */
    int state = 0x12345678;
    
    /* Multiple accumulators to create register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    
    /* Many temporary variables for high register pressure */
    int tmp1, tmp2, tmp3, tmp4, tmp5;
    int tmp6, tmp7, tmp8, tmp9, tmp10;
    int tmp11, tmp12, tmp13, tmp14, tmp15;
    
    /* Volatile modifiers to prevent constant propagation */
    volatile int mod1 = 1103515245;
    volatile int mod2 = 12345;
    volatile int mod3 = 0x5A827999;
    
    /* Main loop with true loop-carried dependency */
    for (int i = 0; i < limit; ++i) {
        /* Loop-carried dependency: state from previous iteration used here */
        state = (state * mod1 + mod2) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        tmp1 = i * mod1;
        tmp2 = i + mod2;
        tmp3 = tmp1 ^ tmp2;
        tmp4 = tmp1 & tmp2;
        tmp5 = tmp1 | tmp2;
        
        tmp6 = state * 3;
        tmp7 = state / 7;
        tmp8 = tmp6 << 2;
        tmp9 = tmp7 >> 1;
        tmp10 = tmp8 ^ tmp9;
        
        tmp11 = acc1 * acc2;
        tmp12 = acc3 + acc4;
        tmp13 = tmp11 & tmp12;
        tmp14 = tmp11 | tmp12;
        tmp15 = tmp13 ^ tmp14;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Memory access with variable indexing */
            int idx = (state ^ i) % size;
            acc1 += data[idx] * mod3;
            acc2 += data[(idx + 1) % size];
        } else {
            acc3 += tmp3 * tmp4;
            acc4 += tmp5 ^ tmp6;
        }
        
        /* More arithmetic to keep variables live */
        acc5 += tmp7 * tmp8;
        acc6 += tmp9 ^ tmp10;
        acc7 += tmp11 & tmp12;
        acc8 += tmp13 | tmp14;
        
        /* Additional loop-carried dependency chain */
        acc1 = (acc1 * 0x9e3779b9) + tmp15;
        acc2 = (acc2 ^ 0x85ebca6b) + state;
        
        /* Complex expression with multiple operators */
        tmp1 = ((acc3 << 3) | (acc4 >> 5)) + ((acc5 * 7) ^ (acc6 & 0xFF));
        tmp2 = ((acc7 ^ acc8) * 11) + ((state >> 16) & 0xFFFF);
        
        /* Update accumulators with cross-dependencies */
        acc3 = tmp1 + i;
        acc4 = tmp2 - i;
        acc5 = acc3 * acc4;
        acc6 = acc5 ^ state;
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    return state + acc1 + acc2 + acc3 + acc4 + acc5 + acc6 + acc7 + acc8;
}

/* Another complex loop with different pattern */
__attribute__((noinline))
int nested_loop(int* data, int size, volatile int outer_limit) {
    int total = 0;
    
    for (int j = 0; j < outer_limit; ++j) {
        volatile int inner_limit = 100;
        int inner_state = j;
        
        for (int k = 0; k < inner_limit; ++k) {
            /* Loop-carried dependency in inner loop */
            inner_state = inner_state * 1664525 + 1013904223;
            
            /* Multiple independent operations */
            int a = inner_state & 0xFF;
            int b = (inner_state >> 8) & 0xFF;
            int c = (inner_state >> 16) & 0xFF;
            int d = (inner_state >> 24) & 0xFF;
            
            int e = a * b + c;
            int f = (d << 3) | (a >> 2);
            int g = e ^ f;
            int h = (b + c) * (d - a);
            
            /* Conditional with memory access */
            if (g & 0x80) {
                total += data[(g + k) % size];
            } else {
                total -= h;
            }
            
            /* Update multiple variables */
            a = b + k;
            b = c * k;
            c = d ^ k;
            d = a & b;
            
            /* Complex expression */
            inner_state = (inner_state + a * b - c * d) & 0x7FFFFFFF;
        }
    }
    
    return total;
}

int main() {
    /* Initialize with pseudo-random data */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand();
    }
    
    /* Volatile loop bounds to prevent optimization */
    volatile int loop_limit = 500;
    volatile int outer_loop_limit = 10;
    
    /* Call complex loops */
    int result1 = complex_loop(data, ARRAY_SIZE, loop_limit);
    int result2 = nested_loop(data, ARRAY_SIZE, outer_loop_limit);
    
    /* Combine and print results to prevent optimization */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Final: %d\n", result1 + result2);
    
    return 0;
}
