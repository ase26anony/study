/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling pass and exercise
 * the dependency edge calculation logic in modulo-sched.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline))
int compute_loop(int *data, int size, volatile int limit) {
    /* High register pressure variables - many live scalars */
    int state = 0x12345678;  /* Loop-carried dependency */
    int sum = 0;
    int acc1 = 1, acc2 = 2, acc3 = 3, acc4 = 4, acc5 = 5;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int a = 7, b = 11, c = 13, d = 17, e = 19;
    volatile int modifier = 3;  /* Prevent constant propagation */
    
    /* Loop with true loop-carried dependency and high complexity */
    for (int i = 0; i < limit; ++i) {
        /* 1. Loop-carried dependency (distance = 1) */
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* 2. Multiple independent arithmetic operations */
        tmp1 = a * i + modifier;
        tmp2 = b + i * c;
        tmp3 = tmp1 ^ tmp2;
        tmp4 = d * state + e;
        tmp5 = tmp3 & tmp4;
        tmp6 = tmp1 | tmp2;
        tmp7 = tmp4 - tmp3;
        tmp8 = tmp5 * tmp6;
        tmp9 = tmp7 / (modifier + 1);
        tmp10 = tmp8 % (e + 1);
        
        /* 3. Memory access with variable indexing */
        int idx = (state + i) % size;
        sum += data[idx] * modifier;
        
        /* 4. Conditional execution based on loop-variant condition */
        if (state & 1) {  /* Nested control flow */
            acc1 += data[(idx + 1) % size];
            tmp1 = tmp1 * 2 + 1;
        } else {
            acc2 ^= data[(idx + 2) % size];
            tmp2 = tmp2 / 2;
        }
        
        /* More arithmetic to increase instruction count */
        acc3 = acc3 * tmp3 + tmp4;
        acc4 = acc4 ^ tmp5 | tmp6;
        acc5 = acc5 + tmp7 - tmp8;
        
        /* Another conditional with different condition */
        if (i & 3) {  /* 75% probability */
            tmp9 = tmp9 * 3;
            acc1 = acc1 ^ tmp10;
        }
        
        /* Update accumulators to keep them live */
        sum = sum + acc1 - acc2 + acc3 + acc4 - acc5;
    }
    
    /* Combine all results to prevent dead code elimination */
    return sum + state + acc1 + acc2 + acc3 + acc4 + acc5;
}

/* Another complex loop with different pattern */
__attribute__((noinline))
int compute_loop2(int *data, int size, volatile int iterations) {
    int prev = 0, curr = 1;  /* Pair with loop-carried dependency */
    int total = 0;
    volatile int scale = 2;
    
    /* Additional high register pressure variables */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5, r6 = 6, r7 = 7, r8 = 8;
    int r9 = 9, r10 = 10, r11 = 11, r12 = 12, r13 = 13, r14 = 14, r15 = 15;
    
    for (int i = 0; i < iterations; ++i) {
        /* Chain of loop-carried dependencies */
        int next = prev + curr + data[i % size];
        prev = curr;
        curr = next;
        
        /* Many parallel computations */
        r1 = r1 * scale + i;
        r2 = r2 ^ data[(i + r1) % size];
        r3 = r3 & r1 | r2;
        r4 = r4 + r3 * 2;
        r5 = r5 - r4 / 3;
        r6 = r6 ^ (r5 << 2);
        r7 = r7 | (r6 >> 1);
        r8 = r8 * r7 + 1;
        r9 = r9 + r8 % 7;
        r10 = r10 ^ r9;
        r11 = r11 & r10;
        r12 = r12 | r11;
        r13 = r13 + r12;
        r14 = r14 - r13;
        r15 = r15 * r14;
        
        /* Conditional with complex expression */
        if ((i + prev) & 7) {
            total += r15 + curr;
            r1 = r1 ^ total;
        } else {
            total -= r15 - curr;
            r2 = r2 | total;
        }
        
        /* Memory access pattern that's hard to predict */
        int offset = (curr * 1103515245) % size;
        total += data[offset] * scale;
    }
    
    return total + prev + curr + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + 
           r9 + r10 + r11 + r12 + r13 + r14 + r15;
}

int main() {
    const int SIZE = 1024;
    int *data = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile loop bounds to prevent optimization */
    volatile int limit1 = 500;
    volatile int limit2 = 400;
    
    /* Call both complex loops */
    int result1 = compute_loop(data, SIZE, limit1);
    int result2 = compute_loop2(data, SIZE, limit2);
    
    /* Combine and print results to prevent dead code elimination */
    int final_result = result1 + result2;
    printf("Final result: %d\n", final_result);
    
    free(data);
    return 0;
}
