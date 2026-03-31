/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling pass and exercise
 * the dependency edge calculation logic in modulo-sched.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline))
int compute_loop(int *data, int size, volatile int limit) {
    /* Multiple accumulators to create register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    int state = 123456789;  /* Loop-carried dependency variable */
    
    /* Volatile variables to prevent constant propagation */
    volatile int mod1 = 17;
    volatile int mod2 = 23;
    volatile int mod3 = 31;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < limit; ++i) {
        /* Loop-carried dependency: state from iteration i used in i+1 */
        int prev_state = state;
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        int tmp1 = acc1 * i;          /* Multiplication */
        int tmp2 = acc2 + i;          /* Addition */
        int tmp3 = tmp1 ^ tmp2;       /* XOR */
        int tmp4 = acc3 & (i * mod1); /* AND with multiplication */
        int tmp5 = acc4 | (i + mod2); /* OR with addition */
        int tmp6 = tmp3 - tmp4;       /* Subtraction */
        int tmp7 = tmp5 * tmp6;       /* Another multiplication */
        int tmp8 = tmp7 % mod3;       /* Modulo operation */
        
        /* More operations to increase instruction count */
        int tmp9 = (acc5 << 2) | (acc6 >> 3);
        int tmp10 = (acc7 * 3) + (acc8 * 5);
        int tmp11 = tmp9 ^ tmp10;
        int tmp12 = tmp11 & 0xFF;
        
        /* Conditional execution based on loop-variant condition */
        if (i & 1) {  /* Simple loop-variant condition */
            /* Memory access with variable indexing */
            int idx = (prev_state + i) % size;
            acc1 += data[idx] * mod1;
            acc2 ^= data[(idx + mod2) % size];
            
            /* More arithmetic in conditional path */
            tmp12 = tmp12 * 2 + 1;
        } else {
            /* Alternative path with different operations */
            acc3 += data[i % size] >> 1;
            acc4 ^= data[(i * 3) % size] & 0x7F;
            tmp12 = tmp12 / 2;
        }
        
        /* Update accumulators to keep them live */
        acc1 = (acc1 + tmp1) & 0xFFFF;
        acc2 = (acc2 ^ tmp2) | 0x1;
        acc3 = (acc3 * 3 + tmp3) % 1000;
        acc4 = (acc4 + tmp4 - tmp5) & 0xFFF;
        acc5 = (acc5 ^ tmp6) + i;
        acc6 = (acc6 * 7 + tmp7) % 256;
        acc7 = (acc7 | tmp8) ^ prev_state;
        acc8 = (acc8 + tmp12) * 11 % 1023;
        
        /* Another conditional with different condition */
        if ((state & 3) == 0) {
            acc1 = acc1 ^ acc2;
            acc3 = acc3 + acc4;
        }
        
        /* Complex expression with multiple operators */
        acc5 = ((acc5 << 1) | (acc6 >> 2)) + ((acc7 & acc8) * 3);
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    int result = acc1 + acc2 + acc3 + acc4 + acc5 + acc6 + acc7 + acc8;
    return result ^ state;  /* Include final state */
}

int main() {
    /* Initialize with pseudo-random data */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile loop limit to prevent constant propagation */
    volatile int iterations = 500;
    
    /* Call the computation loop */
    int result = compute_loop(data, ARRAY_SIZE, iterations);
    
    /* Use the result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Additional computation to ensure multiple loops */
    volatile int small_limit = 100;
    int sum = 0;
    
    /* Another loop with different characteristics */
    for (volatile int j = 0; j < small_limit; ++j) {
        int idx1 = j % ARRAY_SIZE;
        int idx2 = (j * 7) % ARRAY_SIZE;
        int idx3 = (j + 13) % ARRAY_SIZE;
        
        /* Complex addressing calculations */
        sum += data[idx1] * data[idx2] - data[idx3];
        sum ^= data[(idx1 + idx2) % ARRAY_SIZE];
        
        /* More operations to increase complexity */
        if (j & 2) {
            sum += data[idx1] >> 2;
        }
        if (j % 3 == 0) {
            sum ^= 0x55AA;
        }
    }
    
    printf("Final sum: %d\n", sum + result);
    return 0;
}
