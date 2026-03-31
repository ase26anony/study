/* modulo-sched-test.c
 * Test program to trigger GCC's modulo scheduling dependency analysis
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
    volatile int seed = 123;
    
    /* Multiple accumulators to create register pressure */
    int state = seed;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    int acc9 = 0, acc10 = 0, acc11 = 0, acc12 = 0;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state from previous iteration */
        int prev_state = state;
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        int tmp1 = acc1 * i;
        int tmp2 = acc2 + i;
        int tmp3 = tmp1 ^ tmp2;
        int tmp4 = acc3 & i;
        int tmp5 = acc4 | i;
        int tmp6 = tmp3 * tmp4;
        int tmp7 = tmp5 + tmp6;
        int tmp8 = acc5 - i;
        int tmp9 = acc6 * 3;
        int tmp10 = acc7 / 2;
        
        /* Memory access with variable indexing */
        int idx = (i + prev_state) % size;
        int val = data[idx] * 7;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            sum += data[state % size];
            acc1 += val * 2;
        } else {
            acc2 += val / 3;
        }
        
        /* More arithmetic to increase instruction count */
        acc3 = tmp7 + acc8;
        acc4 = tmp8 ^ acc9;
        acc5 = tmp9 | acc10;
        acc6 = tmp10 & acc11;
        
        /* Cross-iteration dependency chain */
        acc7 = acc12 + prev_state;  /* Uses value from previous iteration */
        acc8 = acc1 * 2;
        acc9 = acc2 + 3;
        acc10 = acc3 ^ 4;
        acc11 = acc4 & 5;
        acc12 = acc5 | 6;
        
        /* Additional operations to create more edges in dependency graph */
        int tmp11 = (acc6 << 2) + (acc7 >> 1);
        int tmp12 = (acc8 * acc9) % 100;
        int tmp13 = tmp11 ^ tmp12;
        int tmp14 = (acc10 + acc11) * acc12;
        
        /* Update accumulators with computed values */
        acc1 += tmp13;
        acc2 += tmp14;
        acc3 += tmp11 * tmp12;
        acc4 += tmp13 ^ tmp14;
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    int result = state + sum + acc1 + acc2 + acc3 + acc4 + acc5 + acc6 + 
                 acc7 + acc8 + acc9 + acc10 + acc11 + acc12;
    
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
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Additional loop with different characteristics */
    volatile int limit = 300;
    int x = 0, y = 0, z = 0;
    
    for (int i = 0; i < limit; ++i) {
        /* Another loop-carried dependency */
        int old_x = x;
        x = (y * 13 + i) % 100;
        y = (z * 17 + old_x) % 100;  /* Uses old_x from previous iteration */
        z = (x * 19 + y) % 100;
        
        /* Complex conditional with multiple paths */
        if ((i & 3) == 0) {
            x += data[i % ARRAY_SIZE];
        } else if ((i & 3) == 1) {
            y ^= data[(i + 1) % ARRAY_SIZE];
        } else if ((i & 3) == 2) {
            z |= data[(i + 2) % ARRAY_SIZE];
        } else {
            x &= data[(i + 3) % ARRAY_SIZE];
        }
        
        /* More arithmetic operations */
        int t1 = x * y;
        int t2 = y + z;
        int t3 = z ^ x;
        int t4 = t1 & t2;
        int t5 = t3 | t4;
        int t6 = t5 * 31;
        int t7 = t6 + 17;
        int t8 = t7 % 97;
        
        /* Update variables to keep them live */
        x += t8;
        y -= t7;
        z ^= t6;
    }
    
    printf("Final values: x=%d, y=%d, z=%d\n", x, y, z);
    
    return 0;
}
