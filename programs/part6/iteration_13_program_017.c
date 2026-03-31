/* modulo-sched-test.c
 * Test program to trigger GCC's modulo scheduling dependency analysis
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms -fno-unroll-loops -fno-tree-vectorize modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int complex_loop(int *data, int size, volatile int limit) {
    /* Multiple accumulator variables to create register pressure */
    int state = 123456789;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    int acc6 = 0, acc7 = 0, acc8 = 0, acc9 = 0, acc10 = 0;
    
    /* Loop with true loop-carried dependency: state depends on previous iteration */
    for (int i = 0; i < limit; ++i) {
        /* Loop-carried dependency: state from iteration i used in iteration i+1 */
        int prev_state = state;
        
        /* Multiple independent arithmetic operations */
        int tmp1 = acc1 * i;
        int tmp2 = acc2 + i;
        int tmp3 = tmp1 ^ tmp2;
        int tmp4 = acc3 & i;
        int tmp5 = acc4 | i;
        int tmp6 = acc5 - i;
        int tmp7 = acc6 * 1103515245;
        int tmp8 = acc7 + 12345;
        int tmp9 = acc8 ^ 0x55555555;
        int tmp10 = acc9 << 2;
        
        /* Complex state update with loop-carried dependency */
        state = (prev_state * 1103515245 + 12345) ^ data[i % size];
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Memory access with variable indexing */
            sum += data[(state + i) % size];
            acc1 += data[(i * 3) % size];
        } else {
            acc2 += data[(i * 5) % size];
        }
        
        /* More arithmetic operations to increase instruction count */
        acc3 = tmp3 * 17;
        acc4 = tmp4 + tmp5;
        acc5 = tmp6 ^ tmp7;
        acc6 = tmp8 & tmp9;
        acc7 = tmp10 | state;
        acc8 = acc1 * acc2;
        acc9 = acc3 + acc4;
        acc10 = acc5 ^ acc6;
        
        /* Nested conditional for additional control flow complexity */
        if (i & 3) {
            if (state > 1000) {
                acc7 += data[(acc8 + i) % size];
            } else {
                acc8 -= data[(acc9 - i) % size];
            }
        }
        
        /* Update accumulators to keep them live */
        acc1 = (acc1 + 1) & 0xFFF;
        acc2 = (acc2 * 3) & 0xFFF;
        acc3 = (acc3 ^ acc10) & 0xFFF;
        acc4 = (acc4 | acc9) & 0xFFF;
        acc5 = (acc5 - acc8) & 0xFFF;
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    return sum + state + acc1 + acc2 + acc3 + acc4 + acc5 + 
           acc6 + acc7 + acc8 + acc9 + acc10;
}

/* Another complex loop with different pattern */
__attribute__((noinline))
int another_complex_loop(int *data, int size, volatile int iterations) {
    int result = 0;
    int carry = 1;  /* Loop-carried dependency */
    
    for (int i = 0; i < iterations; ++i) {
        /* Create many temporary variables for register pressure */
        int t1 = data[i % size];
        int t2 = data[(i + 1) % size];
        int t3 = data[(i + 2) % size];
        int t4 = data[(i + 3) % size];
        int t5 = data[(i + 4) % size];
        int t6 = data[(i + 5) % size];
        int t7 = data[(i + 6) % size];
        int t8 = data[(i + 7) % size];
        int t9 = data[(i + 8) % size];
        int t10 = data[(i + 9) % size];
        
        /* Chain of dependent operations */
        int chain1 = t1 * carry;      /* Depends on previous iteration */
        int chain2 = chain1 + t2;
        int chain3 = chain2 * t3;
        int chain4 = chain3 ^ t4;
        int chain5 = chain4 & t5;
        
        /* Parallel independent chains */
        int par1 = t6 * t7;
        int par2 = t8 + t9;
        int par3 = t10 ^ i;
        int par4 = par1 & par2;
        int par5 = par3 | par4;
        
        /* Update loop-carried variable */
        carry = chain5 + par5;
        
        /* Conditional with loop-variant condition */
        if ((i + carry) & 1) {
            result += chain5;
        } else {
            result -= par5;
        }
        
        /* More operations to increase instruction mix */
        result = (result * 13) & 0xFFFFFF;
        carry = (carry * 17) & 0xFFFFFF;
    }
    
    return result + carry;
}

int main() {
    /* Initialize data array with pseudo-random values */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile loop limits to prevent constant propagation */
    volatile int limit1 = 500;
    volatile int limit2 = 300;
    
    /* Execute complex loops */
    int result1 = complex_loop(data, ARRAY_SIZE, limit1);
    int result2 = another_complex_loop(data, ARRAY_SIZE, limit2);
    
    /* Combine and print results to prevent optimization */
    int final_result = result1 + result2;
    printf("Final result: %d\n", final_result);
    
    return 0;
}
