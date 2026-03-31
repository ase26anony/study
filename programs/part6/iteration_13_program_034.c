/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fno-unroll-loops -fno-tree-vectorize -fdump-rtl-sms modulo-sched-test.c -o modulo-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline))
int process_loop(int* data, int size) {
    volatile int N = 500;  /* Prevent constant propagation */
    volatile int seed = 42; /* Volatile to prevent optimization */
    
    /* Many local variables to create register pressure */
    int state = seed;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int a = 3, b = 7, c = 11, d = 13, e = 17, f = 19;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state from previous iteration */
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        tmp1 = a * i;           /* Multiplication */
        tmp2 = b + i;           /* Addition */
        tmp3 = tmp1 ^ tmp2;     /* XOR */
        tmp4 = c & i;           /* Bitwise AND */
        tmp5 = d | i;           /* Bitwise OR */
        tmp6 = tmp4 - tmp5;     /* Subtraction */
        tmp7 = tmp3 * tmp6;     /* Another multiplication */
        tmp8 = e << (i & 3);    /* Shift operation */
        tmp9 = f >> (i & 2);    /* Another shift */
        tmp10 = tmp7 + tmp8 - tmp9; /* Complex expression */
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {  /* Loop-variant condition */
            /* Memory access with variable indexing */
            sum += data[(state + i) % size] * (i & 0xFF);
        } else {
            /* Alternative path with different operations */
            sum -= data[(i * 7) % size] & 0x7F;
        }
        
        /* Update multiple accumulators to keep them live */
        acc1 += tmp1;
        acc2 += tmp2;
        acc3 += tmp3;
        acc4 += tmp10;
        
        /* More arithmetic to increase instruction count */
        a = (a + 1) & 0xF;
        b = (b * 3) & 0xFF;
        c = (c ^ i) & 0x7F;
        d = (d - (i & 1)) & 0x3F;
        e = (e << 1) | (i & 1);
        f = (f >> 1) ^ (i & 0xF);
        
        /* Another loop-carried dependency chain */
        acc1 = acc1 * 6364136223846793005ULL + 1442695040888963407ULL;
    }
    
    /* Combine all results to prevent dead code elimination */
    return sum + acc1 + acc2 + acc3 + acc4 + state;
}

/* Another complex loop with different characteristics */
__attribute__((noinline))
int nested_loop(int* data, int size) {
    volatile int outer_limit = 10;
    volatile int inner_limit = 50;
    int result = 0;
    
    for (int i = 0; i < outer_limit; ++i) {
        int local_state = i * 100;
        
        for (int j = 0; j < inner_limit; ++j) {
            /* Complex addressing with multiple dependencies */
            int idx = (i * 31 + j * 7) % size;
            
            /* Multiple operations with loop-carried dependencies */
            local_state = local_state * 1664525 + 1013904223;
            
            /* Several independent calculations */
            int t1 = data[idx] * j;
            int t2 = data[(idx + 1) % size] + i;
            int t3 = t1 & t2;
            int t4 = t1 | t2;
            int t5 = t3 ^ t4;
            int t6 = t5 << (j & 3);
            int t7 = t6 >> (i & 1);
            
            /* Conditional with nested condition */
            if ((i + j) & 1) {
                if (local_state > 0) {
                    result += t7;
                } else {
                    result -= t7;
                }
            }
            
            /* More temporary variables for pressure */
            int t8 = result * 3;
            int t9 = t8 / 2;
            int t10 = t9 % 256;
            result = t10;
        }
    }
    
    return result;
}

int main() {
    /* Initialize with pseudo-random data */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Process multiple loops to increase scheduling opportunities */
    int result1 = process_loop(data, ARRAY_SIZE);
    int result2 = nested_loop(data, ARRAY_SIZE);
    
    /* Final computation to ensure all work is observable */
    int final_result = result1 + result2;
    
    printf("Result: %d\n", final_result);
    
    /* Additional loop in main to potentially trigger scheduling */
    volatile int extra_iters = 100;
    int extra_sum = 0;
    
    for (int i = 0; i < extra_iters; ++i) {
        /* Mix of operations */
        extra_sum += data[i % ARRAY_SIZE] * i;
        extra_sum ^= data[(i * 3) % ARRAY_SIZE];
        extra_sum = (extra_sum << 1) | (extra_sum >> 31); /* Rotate */
        
        /* Dependency chain */
        int chain = i;
        for (int k = 0; k < 5; ++k) {
            chain = chain * 13 + 1;
        }
        extra_sum += chain;
    }
    
    printf("Extra sum: %d\n", extra_sum + final_result);
    
    return 0;
}
