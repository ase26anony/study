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
    int state = 123456789;
    int sum = 0;
    
    /* Many temporary variables for high register pressure */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15;
    
    /* Volatile modifiers to prevent optimizations */
    volatile int mod1 = 7;
    volatile int mod2 = 13;
    volatile int mod3 = 31;
    
    /* Main loop with true loop-carried dependency */
    for (int i = 0; i < limit; ++i) {
        /* Loop-carried dependency: state depends on previous iteration */
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        t1 = i * mod1;
        t2 = i + mod2;
        t3 = t1 ^ t2;
        t4 = t1 & t2;
        t5 = t1 | t2;
        t6 = t3 * t4;
        t7 = t5 + t6;
        t8 = t7 << 2;
        t9 = t8 >> 1;
        t10 = t9 & 0xFF;
        
        /* More operations using the loop-carried state */
        t11 = state * mod3;
        t12 = t11 + i;
        t13 = t12 ^ t10;
        t14 = t13 * 17;
        t15 = t14 % 256;
        
        /* Conditional execution based on loop-variant condition */
        if (i & 1) {  /* Simple loop-variant condition */
            /* Memory access with variable indexing */
            int idx = (state + i) % size;
            idx = idx < 0 ? -idx : idx;  /* Ensure positive index */
            acc1 += data[idx] * t15;
            
            /* More arithmetic in conditional path */
            t1 = acc1 * 3;
            t2 = t1 + 5;
            acc2 ^= t2;
        } else {
            /* Alternative path with different operations */
            acc3 += t15 * (i % 16);
            t3 = acc3 & 0xFFFF;
            t4 = t3 * 19;
            acc4 |= t4;
        }
        
        /* Cross-iteration dependency through accumulators */
        if (i > 0) {
            /* Use previous iteration's t15 value (simulated) */
            int prev_val = t15 - (i % 3);
            acc1 += prev_val;
        }
        
        /* More operations to increase instruction count */
        t5 = acc1 + acc2;
        t6 = acc3 - acc4;
        t7 = t5 * t6;
        t8 = t7 % 100;
        t9 = t8 + i;
        t10 = t9 ^ state;
        
        /* Update accumulators to keep them live */
        acc1 = (acc1 + t10) & 0x7FFFFFFF;
        acc2 = (acc2 ^ t9) & 0x7FFFFFFF;
        acc3 = (acc3 * 3 + t8) & 0x7FFFFFFF;
        acc4 = (acc4 | t7) & 0x7FFFFFFF;
        
        /* Another memory access with complex addressing */
        int idx2 = (i * 3 + state) % size;
        sum += data[idx2] * (i % 8 + 1);
    }
    
    /* Combine all results to prevent dead code elimination */
    int result = acc1 + acc2 + acc3 + acc4 + sum + state;
    return result;
}

int main() {
    /* Initialize random seed */
    srand(time(NULL));
    
    /* Create data array */
    const int SIZE = 1024;
    int* data = (int*)malloc(SIZE * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with pseudo-random values */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile loop limit to prevent constant propagation */
    volatile int N = 500;
    
    /* Call the complex loop function */
    int result = complex_loop(data, SIZE, N);
    
    /* Use the result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Additional loop with different characteristics */
    {
        volatile int M = 300;
        int temp = 0;
        int prev = 0;
        
        /* Another loop with different dependency pattern */
        for (int i = 0; i < M; ++i) {
            int curr = data[i % SIZE] * 3;
            temp += curr + prev;  /* Loop-carried: uses prev from last iteration */
            prev = curr;
            
            /* More operations */
            int x = temp & 0xFF;
            int y = x * i;
            int z = y ^ prev;
            temp = z % 1000;
        }
        
        printf("Additional result: %d\n", temp);
    }
    
    free(data);
    return 0;
}
