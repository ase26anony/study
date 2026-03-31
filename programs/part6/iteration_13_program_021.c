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
    volatile int N = 500;  /* Prevent constant propagation */
    volatile int mod1 = 7, mod2 = 13, mod3 = 17; /* Volatile modifiers */
    
    /* Multiple accumulator variables to create register pressure */
    int state = 123456789;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    int acc6 = 0, acc7 = 0, acc8 = 0, acc9 = 0, acc10 = 0;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state depends on previous iteration */
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        int tmp1 = acc1 * i;
        int tmp2 = acc2 + i;
        int tmp3 = tmp1 ^ tmp2;
        int tmp4 = acc3 & (i << 2);
        int tmp5 = acc4 | (i * mod1);
        int tmp6 = acc5 - (i % mod2);
        int tmp7 = tmp3 * tmp4;
        int tmp8 = tmp5 + tmp6;
        int tmp9 = tmp7 ^ tmp8;
        int tmp10 = acc6 * mod3;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Memory access with variable indexing */
            sum += data[(state + i) % size];
            acc1 += data[(i * mod1) % size];
        } else {
            acc2 ^= data[(i + mod2) % size];
        }
        
        /* Additional conditional with different condition */
        if ((i & 3) == 0) {
            acc3 = acc3 * 2 + data[i % size];
        }
        
        /* More arithmetic to keep variables live */
        acc4 = tmp9 + acc7;
        acc5 = tmp10 - acc8;
        acc6 = acc9 * acc10;
        acc7 = acc1 ^ acc2;
        acc8 = acc3 | acc4;
        acc9 = acc5 & acc6;
        acc10 = acc7 + acc8;
        
        /* Another loop-carried dependency chain */
        acc1 = acc1 + state;
        acc2 = acc2 ^ (state >> 1);
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    int result = sum + state + acc1 + acc2 + acc3 + acc4 + acc5 + 
                 acc6 + acc7 + acc8 + acc9 + acc10;
    
    return result;
}

int main() {
    /* Initialize array with pseudo-random values */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Process the loop */
    int result = process_loop(data, ARRAY_SIZE);
    
    /* Print result to ensure computation is not optimized away */
    printf("Result: %d\n", result);
    
    /* Additional loop with different characteristics */
    volatile int limit = 300;
    int x = 0, y = 0, z = 0;
    
    for (int i = 0; i < limit; ++i) {
        /* Complex dependency chain */
        int a = x * y;
        int b = y + z;
        int c = a ^ b;
        int d = c * i;
        int e = d & 0xFF;
        
        /* Conditional update with memory access */
        if (i % 2) {
            x = data[e % ARRAY_SIZE] + 1;
        } else {
            y = data[(e + i) % ARRAY_SIZE] - 1;
        }
        
        /* Another dependency chain */
        z = (z * 1664525 + 1013904223) & 0x7FFFFFFF;
        
        /* Multiple parallel operations */
        int f = x * 3;
        int g = y * 5;
        int h = z * 7;
        int j = f + g;
        int k = h ^ j;
        int l = k * 11;
        
        /* Update accumulators */
        x = x + (l & 1);
        y = y ^ (l >> 1);
        z = z | (l >> 2);
    }
    
    printf("Final values: x=%d, y=%d, z=%d\n", x, y, z);
    
    return 0;
}
