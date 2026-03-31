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
int process_loop(int *data, int size) {
    /* Volatile variables to prevent optimization */
    volatile int N = 500;
    volatile int seed_mod = 1103515245;
    volatile int seed_add = 12345;
    
    /* Many local variables to create register pressure */
    int state = 1;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int a = 3, b = 7, c = 11, d = 13, e = 17, f = 19, g = 23, h = 29;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state from previous iteration */
        state = (state * seed_mod + seed_add) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        tmp1 = a * i + b;
        tmp2 = c & i | d;
        tmp3 = e ^ i * f;
        tmp4 = g + i * h;
        tmp5 = tmp1 * tmp2;
        tmp6 = tmp3 | tmp4;
        tmp7 = tmp5 ^ tmp6;
        tmp8 = tmp7 + state;
        tmp9 = tmp8 * a;
        tmp10 = tmp9 >> 2;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Memory access with variable indexing */
            sum += data[(state + i) % size] * (i & 0xF);
        } else {
            /* Alternative computation path */
            sum -= data[(i * 3) % size] >> 1;
        }
        
        /* Update multiple accumulators to keep them live */
        acc1 += tmp1;
        acc2 += tmp2;
        acc3 += tmp3;
        acc4 += tmp4;
        acc5 += tmp5;
        
        /* More operations to increase instruction count */
        a = (a + 1) & 0xFF;
        b = (b * 3) & 0xFF;
        c = (c ^ i) & 0xFF;
        d = (d + tmp10) & 0xFF;
        
        /* Another loop-carried dependency chain */
        acc1 = acc1 * 0x9E3779B9 + acc2;
        acc2 = acc2 * 0x9E3779B9 + acc3;
    }
    
    /* Combine all results to prevent dead code elimination */
    return sum + state + acc1 + acc2 + acc3 + acc4 + acc5 + 
           tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8 + tmp9 + tmp10;
}

int main() {
    /* Initialize array with pseudo-random data */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Process the loop multiple times */
    int total_result = 0;
    volatile int iterations = 3;
    
    for (int j = 0; j < iterations; ++j) {
        total_result += process_loop(data, ARRAY_SIZE);
    }
    
    printf("Result: %d\n", total_result);
    
    return 0;
}
