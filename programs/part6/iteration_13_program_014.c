/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * with loop-carried dependencies and complex instruction scheduling
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int complex_loop(int* data, int size) {
    volatile int N = 500;  /* Prevent constant propagation */
    volatile int mod1 = 7, mod2 = 13;  /* Volatile modifiers */
    
    /* Multiple accumulator variables for high register pressure */
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
        int tmp2 = acc2 + (i ^ mod1);
        int tmp3 = tmp1 ^ tmp2;
        int tmp4 = acc3 & (state >> 3);
        int tmp5 = acc4 | (tmp3 * 2);
        int tmp6 = tmp4 - tmp5;
        int tmp7 = acc5 ^ (tmp6 % 17);
        int tmp8 = acc6 * 3 + tmp7;
        int tmp9 = acc7 & ~tmp8;
        int tmp10 = acc8 | (tmp9 << 2);
        
        /* Memory access with variable indexing */
        int idx = (state + i) % size;
        sum += data[idx] * mod2;
        
        /* Nested control flow */
        if (state & 1) {
            /* Conditional memory access */
            int cond_idx = (state ^ i) % size;
            acc1 += data[cond_idx] * 3;
            tmp10 = tmp10 ^ data[(cond_idx + 1) % size];
        } else {
            acc2 += data[(i * 2) % size] / 2;
        }
        
        /* More arithmetic to increase instruction count */
        acc3 = tmp3 * tmp4;
        acc4 = tmp5 + tmp6;
        acc5 = tmp7 ^ tmp8;
        acc6 = tmp9 - tmp10;
        acc7 = acc1 * acc2;
        acc8 = acc3 + acc4;
        acc9 = acc5 ^ acc6;
        acc10 = acc7 & acc8;
        
        /* Another loop-carried dependency chain */
        acc1 = acc1 ^ state;
        acc2 = acc2 + (state >> 1);
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    int result = sum + state + acc1 + acc2 + acc3 + acc4 + acc5 + 
                 acc6 + acc7 + acc8 + acc9 + acc10;
    
    return result;
}

/* Another complex loop with different pattern */
__attribute__((noinline))
int second_loop(int* data, int size) {
    volatile int M = 300;
    int x = 1, y = 2, z = 3;
    int a = 0, b = 0, c = 0, d = 0, e = 0;
    
    for (int i = 0; i < M; ++i) {
        /* Different loop-carried dependency pattern */
        x = y * z + data[i % size];
        y = z ^ x;
        z = x - y + data[(i + 1) % size];
        
        /* Complex arithmetic network */
        a = b * c + d;
        b = c ^ e + a;
        c = d & e * b;
        d = e | a ^ c;
        e = a + b - c * d;
        
        /* Conditional with arithmetic */
        if ((i & 3) == 0) {
            a += data[(i * 3) % size];
            b ^= data[(i * 5) % size];
        } else if ((i & 3) == 1) {
            c *= data[(i * 7) % size] + 1;
            d -= data[(i * 11) % size];
        }
        
        /* More operations to increase scheduling complexity */
        int t1 = a * i;
        int t2 = b + i;
        int t3 = c ^ i;
        int t4 = d & i;
        int t5 = e | i;
        
        a = t1 ^ t2;
        b = t3 + t4;
        c = t5 * t1;
        d = t2 & t3;
        e = t4 | t5;
    }
    
    return x + y + z + a + b + c + d + e;
}

int main() {
    /* Initialize with pseudo-random data */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Execute complex loops */
    int result1 = complex_loop(data, ARRAY_SIZE);
    int result2 = second_loop(data, ARRAY_SIZE);
    
    /* Use results to prevent optimization */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Combined: %d\n", result1 ^ result2);
    
    return 0;
}
