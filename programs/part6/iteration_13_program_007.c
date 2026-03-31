/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fno-unroll-loops -fno-tree-vectorize -fdump-rtl-sms -fdump-rtl-sched1 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int process_loop(int *data, int size) {
    volatile int N = 500;  /* Prevent constant propagation */
    volatile int seed = 42;
    
    /* Many local variables to create register pressure */
    int state = seed;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int a = 1103515245, b = 12345, c = 67890, d = 54321;
    int e = 255, f = 4095, g = 65535, h = 16777215;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state from iteration i used in i+1 */
        state = (state * a + b) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        tmp1 = state * c;
        tmp2 = state + d;
        tmp3 = tmp1 ^ tmp2;
        tmp4 = tmp1 & e;
        tmp5 = tmp2 | f;
        tmp6 = tmp3 * tmp4;
        tmp7 = tmp5 + tmp6;
        tmp8 = tmp7 >> 2;
        tmp9 = tmp8 * i;
        tmp10 = tmp9 % 997;
        
        /* Memory access with variable indexing */
        if (state & 1) {
            sum += data[(state + i) % size];
        }
        
        /* More arithmetic to increase instruction count */
        acc1 += tmp1;
        acc2 ^= tmp2;
        acc3 |= tmp3;
        acc4 &= tmp4;
        
        /* Conditional execution based on loop-variant condition */
        if (i & 3) {
            tmp5 = tmp6 * tmp7;
            acc1 ^= tmp5;
        } else {
            tmp8 = tmp9 + tmp10;
            acc2 += tmp8;
        }
        
        /* Additional operations to create more dependencies */
        tmp1 = (tmp1 * 3) / 2;
        tmp2 = (tmp2 + 7) & g;
        tmp3 = (tmp3 | h) ^ i;
        
        /* Use all accumulators to keep them live */
        sum += acc1 + acc2 + acc3 + acc4;
    }
    
    /* Combine all results to prevent optimization */
    return sum + state + acc1 + acc2 + acc3 + acc4;
}

/* Another complex loop function */
__attribute__((noinline))
int process_loop2(int *data, int size) {
    volatile int M = 300;
    int prev = 0, curr = 1;
    int result = 0;
    
    /* Variables for register pressure */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    int r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0;
    
    for (int i = 0; i < M; ++i) {
        /* Strong loop-carried dependency chain */
        int next = curr + prev + data[i % size];
        prev = curr;
        curr = next;
        
        /* Many parallel computations */
        r1 = data[(i * 7) % size] * 3;
        r2 = data[(i * 13) % size] + 5;
        r3 = r1 & r2;
        r4 = r1 | r2;
        r5 = r3 ^ r4;
        r6 = r5 * i;
        r7 = r6 >> 1;
        r8 = r7 + curr;
        r9 = r8 % 1023;
        r10 = r9 * prev;
        
        /* Conditional with loop-variant condition */
        if ((i + curr) & 1) {
            result += r1 + r3 + r5 + r7 + r9;
        } else {
            result += r2 + r4 + r6 + r8 + r10;
        }
        
        /* More operations to create scheduling opportunities */
        r1 = (r1 * 1103515245) + 12345;
        r2 = (r2 ^ 0x5A5A5A5A) * 3;
        r3 = r3 + (i << 2);
        r4 = r4 | (0xFF << (i & 7));
    }
    
    return result + curr + prev;
}

int main() {
    /* Initialize data array with pseudo-random values */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Process loops multiple times */
    int total = 0;
    volatile int iterations = 3;
    
    for (int iter = 0; iter < iterations; ++iter) {
        total += process_loop(data, ARRAY_SIZE);
        total += process_loop2(data, ARRAY_SIZE);
        
        /* Modify data slightly each iteration */
        for (int i = 0; i < ARRAY_SIZE; i += 17) {
            data[i] = (data[i] * 13 + 7) % 1000;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
