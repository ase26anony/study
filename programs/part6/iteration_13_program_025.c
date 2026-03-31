/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * with loop-carried dependencies and complex instruction mix
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int complex_loop(int* data, int size, int init) {
    volatile int N = 500;          /* Prevent constant propagation */
    volatile int mod1 = 7;
    volatile int mod2 = 13;
    
    /* Loop-carried state variables */
    int state = init;
    int prev_state = 0;
    
    /* Multiple accumulators to create register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    int acc9 = 0, acc10 = 0, acc11 = 0, acc12 = 0;
    
    /* Temporary variables for independent operations */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
    int tmp7, tmp8, tmp9, tmp10, tmp11, tmp12;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state depends on previous iteration */
        prev_state = state;  /* Use from previous iteration */
        state = (prev_state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        tmp1 = acc1 * i;                     /* Multiplication */
        tmp2 = acc2 + (i & 0xFF);            /* Addition with mask */
        tmp3 = tmp1 ^ tmp2;                  /* XOR operation */
        tmp4 = acc3 | (i << 2);              /* Bitwise OR with shift */
        tmp5 = acc4 & (data[(i + 1) % size]); /* Bitwise AND with memory */
        tmp6 = tmp3 * tmp4;                  /* Another multiplication */
        
        /* More operations using different operators */
        tmp7 = (acc5 + mod1) * (acc6 - mod2);
        tmp8 = (tmp5 >> 3) | (tmp6 << 1);
        tmp9 = (tmp7 & 0xFFFF) + (tmp8 & 0xFFFF);
        tmp10 = tmp9 * 16807 % 2147483647;
        tmp11 = (tmp10 ^ state) + i;
        tmp12 = tmp11 * (i + 1) / (mod1 + 1);
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {  /* Branch inside loop */
            /* Memory access with variable indexing */
            int idx = (state + i) % size;
            acc1 += data[idx] * 3;
            acc2 ^= data[(idx + mod1) % size];
        } else {
            acc3 |= data[(i * mod2) % size];
            acc4 &= data[(state >> 2) % size];
        }
        
        /* Additional conditional with different test */
        if ((i & 3) == 0) {
            tmp1 = data[(i + acc5) % size] + tmp12;
            acc5 = (acc5 * 13 + tmp1) % 1000;
        }
        
        /* Update multiple accumulators to keep them live */
        acc6 = acc6 + tmp3 - tmp4;
        acc7 = acc7 ^ tmp5 ^ tmp6;
        acc8 = acc8 | tmp7 | tmp8;
        acc9 = acc9 * 3 + tmp9;
        acc10 = (acc10 + tmp10) % 10007;
        acc11 = acc11 ^ tmp11 ^ tmp12;
        acc12 = (acc12 + i * state) & 0x7FFFFFFF;
        
        /* Another loop-carried dependency chain */
        if (i > 0) {
            int prev_acc = acc1;  /* Use from previous iteration */
            acc1 = (prev_acc + acc2) * 2 - acc3;
        }
    }
    
    /* Combine all results to prevent optimization */
    int result = state + prev_state + acc1 + acc2 + acc3 + acc4 + 
                 acc5 + acc6 + acc7 + acc8 + acc9 + acc10 + 
                 acc11 + acc12 + tmp1 + tmp2;
    
    return result;
}

int main() {
    /* Initialize with pseudo-random data */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile initializer to prevent constant propagation */
    volatile int init_val = rand() % 100;
    
    /* Execute the complex loop */
    int result = complex_loop(data, ARRAY_SIZE, init_val);
    
    /* Print result to ensure computation isn't optimized away */
    printf("Result: %d\n", result);
    
    /* Additional loop with different characteristics */
    {
        volatile int limit = 300;
        int sum = 0;
        int carry = 1;
        
        for (int i = 0; i < limit; ++i) {
            /* Different loop-carried dependency pattern */
            int old_carry = carry;
            carry = (old_carry * data[i % ARRAY_SIZE]) % 97 + i;
            
            /* Complex address calculation */
            int idx1 = (i * 17 + old_carry) % ARRAY_SIZE;
            int idx2 = (i * 23 + carry) % ARRAY_SIZE;
            int idx3 = (i * 37 + sum) % ARRAY_SIZE;
            
            /* Multiple memory accesses with dependencies */
            int val1 = data[idx1] + data[idx2];
            int val2 = data[idx3] * old_carry;
            int val3 = (val1 & 0xFF) | (val2 & 0xFF00);
            
            /* Chain of dependent operations */
            sum = sum + val1 - val2 + val3;
            
            /* More temporary variables for pressure */
            int t1 = sum * i, t2 = carry + i, t3 = t1 ^ t2;
            int t4 = val3 | t3, t5 = t4 << 2, t6 = t5 >> 1;
            
            /* Conditional update */
            if (t6 > 1000) {
                sum = sum / 2 + t6;
            }
        }
        
        printf("Secondary result: %d\n", sum + result);
    }
    
    return 0;
}
