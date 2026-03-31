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
long process_loop(int *data, int n) {
    volatile int limit = n;  /* Prevent constant propagation */
    volatile int seed = 123;
    
    /* Loop-carried state with true dependency (distance = 1) */
    long state = seed;
    
    /* Multiple accumulators to create register pressure */
    long acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    long acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    
    /* Many temporary variables for high register pressure */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int tmp11, tmp12, tmp13, tmp14, tmp15;
    
    volatile int mod1 = 7, mod2 = 13, mod3 = 17;  /* Prevent optimization */
    
    for (int i = 0; i < limit; ++i) {
        /* ===== LOOP-CARRIED DEPENDENCY (distance = 1) ===== */
        /* State from iteration i used in iteration i+1 */
        state = (state * 1103515245 + 12345) ^ data[i];
        
        /* ===== MULTIPLE INDEPENDENT ARITHMETIC OPERATIONS ===== */
        /* These can potentially be scheduled in parallel */
        tmp1 = i * mod1;
        tmp2 = i + mod2;
        tmp3 = tmp1 ^ tmp2;          /* Independent operation 1 */
        tmp4 = tmp1 & tmp2;          /* Independent operation 2 */
        tmp5 = tmp1 | tmp2;          /* Independent operation 3 */
        tmp6 = tmp1 * tmp2;          /* Independent operation 4 */
        
        /* More arithmetic with different operators */
        tmp7 = (tmp3 << 2) + tmp4;
        tmp8 = (tmp5 >> 1) * tmp6;
        tmp9 = tmp7 % 31;
        tmp10 = tmp8 & 0xFF;
        
        /* Complex expression with multiple operations */
        tmp11 = (tmp9 * tmp10) + (i % 19);
        tmp12 = (tmp11 ^ state) * 3;
        tmp13 = tmp12 + (state >> 4);
        tmp14 = tmp13 - (i & 15);
        tmp15 = tmp14 | (state & 255);
        
        /* ===== MEMORY ACCESS WITH VARIABLE INDEXING ===== */
        /* Array access with loop-variant index */
        int idx = (i + state) % ARRAY_SIZE;
        int val = data[idx] * mod3;
        
        /* ===== CONDITIONAL CONTROL FLOW ===== */
        /* Creates conditional execution paths in scheduling graph */
        if (state & 1) {
            /* Memory-dependent operation when state is odd */
            acc1 += data[(state + i) % ARRAY_SIZE];
            acc2 ^= val * tmp15;
        } else {
            /* Different path when state is even */
            acc3 |= data[(i * 3) % ARRAY_SIZE];
            acc4 &= val + tmp14;
        }
        
        /* Additional conditional with different condition */
        if (i & 3) {
            acc5 += tmp11 * tmp12;
            acc6 -= tmp13 / (tmp14 ? tmp14 : 1);
        }
        
        if ((i + state) & 7) {
            acc7 ^= tmp15 * (i % 11);
            acc8 |= tmp9 + tmp10;
        }
        
        /* Cross-iteration dependency through accumulators */
        acc1 = acc1 ^ acc2;
        acc2 = acc2 + acc3;
        acc3 = acc3 | acc4;
        acc4 = acc4 - acc5;
        
        /* More register pressure operations */
        tmp1 = acc5 + acc6;
        tmp2 = acc7 * acc8;
        tmp3 = tmp1 ^ tmp2;
        tmp4 = state & tmp3;
        
        /* Prevent dead code elimination */
        if ((i % 128) == 0) {
            acc5 = acc5 ^ tmp4;
            acc6 = acc6 + (state % 37);
        }
    }
    
    /* Combine all accumulators to ensure computations are used */
    long result = state + acc1 + acc2 + acc3 + acc4 + 
                  acc5 + acc6 + acc7 + acc8;
    
    return result;
}

/* Another loop with different characteristics */
__attribute__((noinline))
long process_loop2(int *data, int n) {
    volatile int limit = n;
    long sum = 0;
    long prod = 1;
    
    /* Different loop-carried dependency pattern */
    long carry = 1;
    
    for (int i = 0; i < limit; ++i) {
        /* Multi-cycle dependency chain */
        long t1 = carry * data[i];
        long t2 = t1 + (i * 17);
        long t3 = t2 ^ (data[i] >> 3);
        long t4 = t3 & 0xFFFF;
        
        /* Update loop-carried variable */
        carry = t4 + (carry % 256);
        
        /* Multiple independent operations */
        int a = i * 3;
        int b = i + 5;
        int c = a ^ b;
        int d = a & b;
        int e = c | d;
        int f = e * 7;
        int g = f % 19;
        int h = g << 2;
        int j = h >> 1;
        
        /* Conditional with memory access */
        if ((carry + i) & 1) {
            sum += data[(i + carry) % ARRAY_SIZE] * j;
        } else {
            prod *= (data[i % ARRAY_SIZE] + 1) & 0xFF;
        }
        
        /* More operations to increase instruction count */
        sum = sum ^ (prod & 0xFF);
        prod = prod | (sum & 0xFF);
    }
    
    return sum + prod + carry;
}

int main() {
    /* Initialize with pseudo-random data */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 256;
    }
    
    volatile int iterations = 500;  /* Prevent optimization */
    
    /* Process multiple loops to increase scheduling opportunities */
    long result1 = process_loop(data, iterations);
    long result2 = process_loop2(data, iterations / 2);
    
    /* Combine results to prevent optimization */
    long final_result = result1 ^ result2;
    
    /* Use result to prevent dead code elimination */
    printf("Result: %ld\n", final_result);
    
    /* Additional test with different iteration counts */
    volatile int small_iter = 100;
    volatile int large_iter = 1000;
    
    long r3 = process_loop(data, small_iter);
    long r4 = process_loop2(data, large_iter);
    
    printf("Additional results: %ld, %ld\n", r3, r4);
    
    return 0;
}
