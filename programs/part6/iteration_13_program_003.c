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
    volatile int seed = 123;
    
    /* Multiple accumulators to create register pressure */
    int state = seed;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    
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
        int tmp11 = tmp8 << 2;
        int tmp12 = tmp9 >> 1;
        
        /* Complex memory access with variable indexing */
        int idx = (i + prev_state) % size;
        int val = data[idx] * 7;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            sum += data[state % size] * val;
            acc1 += tmp7;
        } else {
            sum -= data[(state * 3) % size];
            acc2 ^= tmp6;
        }
        
        /* Additional conditional with different condition */
        if (i & 3) {
            acc3 = tmp4 + tmp5;
            acc4 = tmp8 * tmp9;
        } else {
            acc5 = tmp10 | tmp11;
            acc6 = tmp12 & tmp1;
        }
        
        /* More arithmetic to increase instruction count */
        acc7 = (acc7 * 13 + i) % 1001;
        acc8 = (acc8 ^ data[(i * 17) % size]) + tmp2;
        
        /* Cross-iteration dependency chain */
        int chain = prev_state;
        chain = chain * 3 + 1;
        chain = chain ^ (chain >> 1);
        acc1 = acc1 ^ chain;
        
        /* Memory store with dependency */
        data[(i * 19) % size] = chain % 256;
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    int result = state + sum + acc1 + acc2 + acc3 + acc4 + 
                 acc5 + acc6 + acc7 + acc8;
    
    return result;
}

/* Another loop with different characteristics */
__attribute__((noinline))
int process_loop2(int *data, int size) {
    volatile int M = 300;
    int total = 0;
    
    for (int i = 0; i < M; ++i) {
        /* Different loop-carried dependency pattern */
        static int carry = 0;
        int new_carry = (carry + data[i % size]) * 636413622;
        carry = new_carry;
        
        /* Many temporary variables for register pressure */
        int t1 = data[(i + 1) % size];
        int t2 = data[(i + 2) % size];
        int t3 = data[(i + 3) % size];
        int t4 = data[(i + 4) % size];
        int t5 = t1 * t2;
        int t6 = t3 + t4;
        int t7 = t5 ^ t6;
        int t8 = t1 & t2;
        int t9 = t3 | t4;
        int t10 = t7 * t8;
        int t11 = t9 - t10;
        int t12 = t11 << (i & 7);
        int t13 = t12 >> 2;
        int t14 = t13 * 137;
        int t15 = t14 % 997;
        
        /* Nested if-else chain */
        if (i & 1) {
            if (carry & 2) {
                total += t15;
            } else {
                total -= t13;
            }
        } else {
            if (i & 4) {
                total ^= t14;
            } else {
                total |= t15;
            }
        }
        
        /* Update array with dependency */
        data[(i * 23) % size] = total % 1000;
    }
    
    return total;
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
    int result2 = process_loop2(data, ARRAY_SIZE);
    
    /* Final computation to ensure results are used */
    int final_result = result1 ^ result2;
    
    /* Print to prevent optimization */
    printf("Result: %d\n", final_result);
    
    /* Additional loop in main to test different context */
    volatile int K = 200;
    int local_acc = 0;
    
    for (int i = 0; i < K; ++i) {
        int a = data[i % ARRAY_SIZE];
        int b = data[(i + 100) % ARRAY_SIZE];
        int c = a * b + i;
        int d = (a ^ b) << (i & 3);
        int e = c * d;
        int f = e % 7919;
        
        if (i & 1) {
            local_acc += f;
        } else {
            local_acc -= f;
        }
        
        /* Address calculation with multiple operations */
        int addr = (i * 29 + local_acc) % ARRAY_SIZE;
        data[addr] = f;
    }
    
    printf("Local accumulator: %d\n", local_acc);
    
    return 0;
}
