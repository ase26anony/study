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
            sum += data[state % size];
            acc1 = val + tmp7;
        } else {
            sum -= data[(state * 3) % size];
            acc1 = val - tmp7;
        }
        
        /* Update multiple accumulators to keep them live */
        acc2 = tmp11 + tmp12;
        acc3 = tmp10 ^ prev_state;
        acc4 = acc8 * data[(i * 2) % size];
        acc5 = tmp6 | tmp9;
        acc6 = tmp3 & tmp4;
        acc7 = tmp5 + tmp8;
        acc8 = tmp1 - tmp2;
        
        /* Additional arithmetic to increase instruction count */
        int tmp13 = (acc1 * 13) / 5;
        int tmp14 = (acc2 + 17) % 19;
        int tmp15 = tmp13 ^ tmp14;
        int tmp16 = (acc3 << 3) | (acc4 >> 2);
        int tmp17 = tmp15 * tmp16;
        
        /* Use results to prevent dead code elimination */
        if (tmp17 > 1000) {
            acc1 += tmp17 % 100;
        }
    }
    
    /* Combine all accumulators */
    int result = state + sum + acc1 + acc2 + acc3 + acc4 + 
                 acc5 + acc6 + acc7 + acc8;
    
    return result;
}

/* Another loop with different characteristics */
__attribute__((noinline))
int process_loop2(int *data, int size) {
    volatile int M = 300;
    int x = 1, y = 2, z = 3;
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    
    for (int i = 0; i < M; ++i) {
        /* Different loop-carried dependency pattern */
        int old_x = x;
        x = y * data[i % size];
        y = z + old_x;
        z = x ^ y;
        
        /* Parallel computations */
        a = b * c + i;
        b = c ^ d - i;
        c = d | e * 3;
        d = e & f + 5;
        e = f << (i % 4);
        f = a >> (i % 3);
        
        /* Memory access with complex addressing */
        int addr = (x + y + z) % size;
        int load1 = data[addr];
        int load2 = data[(addr * 7 + 11) % size];
        
        /* Conditional with both paths used */
        if ((i + load1) & 3) {
            a += load2;
            b -= load1;
        } else {
            a -= load2;
            b += load1;
        }
        
        /* More operations to increase scheduling complexity */
        int t1 = a * 11;
        int t2 = b * 13;
        int t3 = c * 17;
        int t4 = d * 19;
        int t5 = t1 + t2;
        int t6 = t3 - t4;
        int t7 = t5 ^ t6;
        
        c = t7 & 0xFF;
        d = (t7 >> 8) & 0xFF;
    }
    
    return x + y + z + a + b + c + d + e + f;
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
    
    /* Use results to prevent optimization */
    volatile int final_result = result1 + result2;
    printf("Result: %d\n", final_result);
    
    return 0;
}
