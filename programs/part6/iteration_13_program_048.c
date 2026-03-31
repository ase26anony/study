/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fno-unroll-loops -fno-tree-vectorize -fdump-rtl-sms modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline))
int process_loop(int *data, int size) {
    volatile int N = 500;  /* Prevent constant propagation */
    volatile int seed = 12345;
    
    /* Multiple accumulators to create register pressure */
    int state = seed;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int acc1 = 1, acc2 = 2, acc3 = 3, acc4 = 4;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    int var1 = 7, var2 = 11, var3 = 13, var4 = 17;
    int mod1 = 19, mod2 = 23, mod3 = 29, mod4 = 31;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state depends on previous iteration */
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        tmp1 = acc1 * var1 + i;
        tmp2 = acc2 & var2 | i;
        tmp3 = acc3 ^ var3 * i;
        tmp4 = acc4 + var4 - i;
        tmp5 = tmp1 * tmp2 + mod1;
        tmp6 = tmp3 & tmp4 | mod2;
        tmp7 = tmp5 ^ tmp6 * mod3;
        tmp8 = tmp7 + state & mod4;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Memory access with variable indexing */
            sum1 += data[(state + i) % size];
            sum2 ^= data[(tmp8 + i) % size];
        } else {
            sum3 |= data[(tmp1 + i) % size];
            sum4 &= data[(tmp2 * i) % size];
        }
        
        /* Update accumulators with loop-carried dependencies */
        acc1 = (acc1 + tmp1) ^ state;
        acc2 = (acc2 * tmp2) | (i & 0xFF);
        acc3 = (acc3 - tmp3) & (state | 0x7F);
        acc4 = (acc4 ^ tmp4) + (i % 64);
        
        /* More arithmetic to increase instruction count */
        var1 = (var1 * 3) % 100;
        var2 = (var2 + 5) % 200;
        var3 = (var3 ^ tmp5) % 300;
        var4 = (var4 | tmp6) % 400;
        mod1 = (mod1 + tmp7) % 500;
        mod2 = (mod2 * tmp8) % 600;
        mod3 = (mod3 ^ acc1) % 700;
        mod4 = (mod4 & acc2) % 800;
    }
    
    /* Combine all results to prevent optimization */
    return state + sum1 + sum2 + sum3 + sum4 + acc1 + acc2 + acc3 + acc4 +
           tmp1 + tmp2 + tmp3 + tmp4 + var1 + var2 + var3 + var4 +
           mod1 + mod2 + mod3 + mod4;
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
    
    /* Process the loop */
    int result = process_loop(data, SIZE);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    free(data);
    return 0;
}
