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
    int i;
    
    /* Loop-carried state variables */
    int state = 0x12345678;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    /* Many local variables to create register pressure */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    
    /* Volatile modifiers to prevent optimization */
    volatile int mod1 = 1103515245;
    volatile int mod2 = 12345;
    volatile int mod3 = 67890;
    
    for (i = 0; i < N; i++) {
        /* 1. Loop-carried dependency: state depends on previous iteration */
        state = (state * mod1 + mod2) ^ data[i % size];
        
        /* 2. Multiple independent arithmetic operations */
        tmp1 = state * i;          /* Multiplication */
        tmp2 = state + i;          /* Addition */
        tmp3 = tmp1 ^ tmp2;        /* XOR */
        tmp4 = tmp1 & tmp2;        /* AND */
        tmp5 = tmp1 | tmp2;        /* OR */
        tmp6 = tmp3 << 2;          /* Shift */
        tmp7 = tmp4 >> 1;          /* Shift */
        tmp8 = tmp5 * 3;           /* Multiplication */
        tmp9 = tmp6 + tmp7;        /* Addition */
        tmp10 = tmp8 ^ tmp9;       /* XOR */
        
        /* 3. Memory access with variable indexing */
        int idx = (state + i) % size;
        sum += data[idx] * mod3;
        
        /* 4. Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Complex address calculation */
            int alt_idx = (idx * 7 + 13) % size;
            acc1 += data[alt_idx] * i;
        } else {
            acc2 += data[idx] * (i + 1);
        }
        
        /* 5. More arithmetic to increase instruction count */
        v1 = tmp1 * tmp2;
        v2 = tmp3 + tmp4;
        v3 = v1 ^ v2;
        v4 = v1 & v2;
        v5 = v3 | v4;
        v6 = v5 << (i & 3);        /* Variable shift */
        v7 = v6 * state;
        v8 = v7 + tmp10;
        v9 = v8 ^ state;
        v10 = v9 & 0xFF;
        
        /* 6. Update accumulators to keep values live */
        acc3 += v10;
        acc4 += (tmp10 * v10) % 256;
        
        /* 7. Additional loop-carried dependency chain */
        acc1 = (acc1 * 13 + 17) ^ acc2;
        acc2 = (acc2 * 19 + 23) ^ acc1;
    }
    
    /* Combine all results to prevent dead code elimination */
    return sum + acc1 + acc2 + acc3 + acc4 + state;
}

/* Another loop with different characteristics */
__attribute__((noinline))
int process_loop2(int *data, int size) {
    volatile int M = 300;
    int i, j;
    
    int total = 0;
    int prev = 0;
    int curr = 1;
    
    /* Create nested loop structure */
    for (i = 0; i < M; i++) {
        /* Fibonacci-like loop-carried dependency */
        int next = prev + curr;
        prev = curr;
        curr = next;
        
        /* Multiple accumulators */
        int sum1 = 0, sum2 = 0, sum3 = 0;
        
        /* Inner computation with many operations */
        for (j = 0; j < 8; j++) {
            int idx = (i * 8 + j) % size;
            
            /* Independent parallel operations */
            int a = data[idx] * j;
            int b = data[(idx + 1) % size] + j;
            int c = a ^ b;
            int d = a & b;
            int e = c | d;
            int f = e << (j & 3);
            
            sum1 += a;
            sum2 += b;
            sum3 += f;
            
            /* Conditional store */
            if ((i + j) & 1) {
                total += sum1;
            } else {
                total += sum2;
            }
        }
        
        total += sum3 + curr;
    }
    
    return total + prev;
}

int main() {
    int i;
    int data[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Process multiple loops to increase scheduling opportunities */
    int result1 = process_loop(data, ARRAY_SIZE);
    int result2 = process_loop2(data, ARRAY_SIZE);
    
    /* Combine and print results */
    int final_result = result1 + result2;
    printf("Final result: %d\n", final_result);
    
    /* Use result to prevent optimization */
    if (final_result > 0) {
        return 0;
    } else {
        return 1;
    }
}
