/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms -fno-unroll-loops -fno-tree-vectorize modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int complex_loop(int *data, int size) {
    volatile int N = 500;  /* Prevent constant propagation */
    int i, result = 0;
    
    /* Loop-carried state with true dependency (distance = 1) */
    int state = 0x12345678;
    
    /* Many local variables to create register pressure */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, j = 9;
    int k = 10, l = 11, m = 12, n = 13, o = 14, p = 15, q = 16, r = 17;
    
    /* Additional accumulators to keep values live */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    
    for (i = 0; i < N; ++i) {
        /* 1. Loop-carried dependency: state from iteration i used in i+1 */
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* 2. Multiple independent arithmetic operations */
        int tmp1 = a * i + b;
        int tmp2 = c & i | d;
        int tmp3 = e ^ i * f;
        int tmp4 = g + i * h;
        int tmp5 = j - i * k;
        int tmp6 = l * i + m;
        int tmp7 = n | i & o;
        int tmp8 = p ^ i * q;
        int tmp9 = r + i * a;
        
        /* 3. Memory access with variable indexing */
        int idx = (state + i) % size;
        int mem_val = data[idx] * data[(idx + 1) % size];
        
        /* 4. Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Complex memory access pattern */
            acc1 += data[(state + tmp1) % size];
            acc2 ^= mem_val * tmp2;
        } else {
            acc3 |= tmp3 + tmp4;
            acc4 &= tmp5 ^ tmp6;
        }
        
        /* 5. More arithmetic mixing all temporaries */
        acc5 += tmp7 * tmp8 - tmp9;
        
        /* 6. Additional operations creating cross-iteration dependencies */
        a = (a + tmp1) & 0xFFF;
        b = (b ^ tmp2) | 1;
        c = (c * tmp3) % 1000;
        
        /* 7. Another loop-carried dependency chain */
        d = d + data[i % size] * e;
        e = e ^ (f * g);
        
        /* 8. Complex expression with multiple operators */
        result += ((tmp1 * tmp2) + (tmp3 & tmp4) | (tmp5 ^ tmp6)) - mem_val;
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    return result + acc1 + acc2 + acc3 + acc4 + acc5 + state + a + b + c + d + e;
}

int main() {
    const int SIZE = 1024;
    int *data = (int*)malloc(SIZE * sizeof(int));
    int i, final_result;
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile to prevent constant propagation of loop count */
    volatile int iterations = 3;
    
    /* Call multiple times to ensure optimization */
    final_result = 0;
    for (i = 0; i < iterations; ++i) {
        final_result ^= complex_loop(data, SIZE);
    }
    
    printf("Result: %d\n", final_result);
    
    free(data);
    return 0;
}
